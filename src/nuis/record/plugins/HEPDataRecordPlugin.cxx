#include "nuis/record/plugins/HEPDataRecordPlugin.h"

#include "nuis/binning/Binning.h"

#include "nuis/record/Utility.h"
#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/SingleDistributionAnalysis.h"
#include "nuis/record/SingleFluxAnalysis.h"

#include "nuis/env.h"
#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/HEPData/TableFactory.h"

#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/core.h"
#include "fmt/ranges.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <filesystem>

using namespace nuis;
using namespace ps;

namespace nuis {

DECLARE_NUISANCE_EXCEPT(InvalidAnalysisForRecord);
DECLARE_NUISANCE_EXCEPT(InvalidRecordForAnalysisType);

HEPDataRecordPlugin::HEPDataRecordPlugin(YAML::Node const &cfg) {
  node = cfg;

  HEPData::ResourceReference recordref;

  if (cfg["recordref"]) {
    recordref = HEPData::ResourceReference(cfg["recordref"].as<std::string>());
  } else if (cfg["recordpath"]) {
    recordref =
        HEPData::PathResourceReference(cfg["recordpath"].as<std::string>());
  }

  log_debug("Reading hepdata-format record from reference: {}",
            recordref.str());

  record = HEPData::make_Record(recordref, nuis::env::NUISANCEDB());
}

std::vector<std::string> HEPDataRecordPlugin::get_analyses() const {
  std::vector<std::string> table_names;
  for (auto const &xsm : record.measurements) {
    table_names.push_back(xsm.name);
  }
  return table_names;
}

auto get_measurement(HEPData::Record const &record, std::string const &name) {
  for (auto const &xsm : record.measurements) {
    if (name == xsm.name) {
      return xsm;
    }
  }

  std::vector<std::string> table_names;
  for (auto const &xsm : record.measurements) {
    table_names.push_back(xsm.name);
  }

  log_critical("requested analysis table, named \"{}\", does not exist on "
               "record {}. Valid "
               "tables: {}",
               name, record.record_ref.str(), table_names);
  throw InvalidAnalysisForRecord()
      << "analysis table, named " << name
      << ", does not exist on record: " << record.record_ref.str();
}

auto to_extents(std::vector<HEPData::Variable> const &independent_vars) {
  std::vector<Binning::BinExtents> extents;

  for (size_t bin_i = 0; bin_i < independent_vars.front().values.size();
       ++bin_i) {
    extents.emplace_back();
    for (size_t proj_i = 0; proj_i < independent_vars.size(); ++proj_i) {
      auto const &[low, high] = std::get<HEPData::Extent>(
          independent_vars[proj_i].values[bin_i].value);
      extents.back().emplace_back(low, high);
    }
  }

  return extents;
}

auto get_units_scale(std::set<std::string> const &units_bits,
                     IAnalysis::Target const &stgt) {
  NuHepMC::CrossSection::Units::Unit unit{
      NuHepMC::CrossSection::Units::Scale::CustomType,
      NuHepMC::CrossSection::Units::TargetScale::CustomType};

  double extra_target_scale = 1;

  for (auto const &u : units_bits) {

    auto su = NuHepMC::GR6::ParseCrossSectionScaleUnits(u);
    if (su != NuHepMC::CrossSection::Units::Scale::CustomType) {
      unit.scale = su;
      continue;
    }

    auto tsu = NuHepMC::GR6::ParseCrossSectionTargetScaleUnits(u);
    if (tsu != NuHepMC::CrossSection::Units::TargetScale::CustomType) {
      unit.tgtscale = tsu;
    } else if ((u == "PerTargetNeutron")) {
      unit.tgtscale = NuHepMC::CrossSection::Units::TargetScale::PerNucleon;
      extra_target_scale = stgt.A / stgt.N;
    } else if ((u == "PerTargetProton")) {
      unit.tgtscale = NuHepMC::CrossSection::Units::TargetScale::PerNucleon;
      extra_target_scale = stgt.A / stgt.Z;
    }
  }

  return std::make_pair(unit, extra_target_scale);
}

BinnedValues ProbeFluxToBinnedValuesCount(HEPData::ProbeFlux const &pf) {

  BinnedValues flux_hist(Binning::brute_force(to_extents(pf.independent_vars)),
                         pf.source.filename().native());

  flux_hist.resize();
  for (size_t bin_i = 0; bin_i < pf.dependent_vars[0].values.size(); ++bin_i) {
    flux_hist.values(bin_i, 0) =
        std::get<double>(pf.dependent_vars[0].values[bin_i].value);
  }

  if (pf.bin_content_type == "count_density") {
    return ToCount(flux_hist);
  }
  return flux_hist;
}

int probe_particle_to_pdg(std::string const &probe) {
  try {
    auto pid = std::stol(probe);
    return pid;
  } catch (std::invalid_argument &ia) {
  }

  if (probe == "numu") {
    return 14;
  } else if (probe == "numubar") {
    return -14;
  } else if (probe == "nue") {
    return 12;
  } else if (probe == "nuebar") {
    return -12;
  }

  throw std::runtime_error(fmt::format(
      "unable to parse probe specification string to pdg code: {}", probe));
}

Eigen::MatrixXd mat_from_table(HEPData::Table const &tbl, int nbins) {
  Eigen::MatrixXd mat = Eigen::MatrixXd::Zero(nbins, nbins);

  if (tbl.independent_vars.size() != 2) {
    throw std::runtime_error(
        fmt::format("when trying to convert HEPData::Table to Eigen::MatrixXd, "
                    "have an invalid number of independent variables: {}",
                    tbl.independent_vars.size()));
  }

  for (size_t i = 0; i < tbl.dependent_vars[0].values.size(); ++i) {
    int bin_i = std::get<double>(tbl.independent_vars[0].values[i].value);
    int bin_j = std::get<double>(tbl.independent_vars[1].values[i].value);

    if (bin_i >= nbins) {
      throw std::runtime_error(fmt::format(
          "when trying to convert HEPData::Table to Eigen::MatrixXd, "
          "have bin_i = {}, but nbins expected is {}",
          bin_i, nbins));
    }
    if (bin_j >= nbins) {
      throw std::runtime_error(fmt::format(
          "when trying to convert HEPData::Table to Eigen::MatrixXd, "
          "have bin_j = {}, but nbins expected is {}",
          bin_j, nbins));
    }

    mat(bin_i, bin_j) = std::get<double>(tbl.dependent_vars[0].values[i].value);
  }

  return mat;
}

AnalysisPtr HEPDataRecordPlugin::make_SingleDistributionAnalysis(
    HEPData::CrossSectionMeasurement const &xsmeasurement) {
  auto const &ivars = xsmeasurement.independent_vars;
  auto const &dvar = xsmeasurement.dependent_vars[0];

  log_trace("make_SingleDistributionAnalysis({})",
            xsmeasurement.source.native());

  auto analysis = std::make_shared<SingleDistributionAnalysis>();

  // load all of the proselecta source files
  std::set<std::filesystem::path> proselecta_sources;
  proselecta_sources.insert(xsmeasurement.get_single_selectfunc().source);
  for (auto const &projfs : xsmeasurement.get_single_projectfuncs()) {
    proselecta_sources.insert(projfs.source);
  }

  for (auto const &src : proselecta_sources) {
    log_trace("ProSelecta::Get().load_file({})", src.native());
    ProSelecta::Get().load_file(src);
  }

  analysis->selection = {xsmeasurement.get_single_selectfunc().fname,
                         ProSelecta::Get().get_select_func(
                             xsmeasurement.get_single_selectfunc().fname,
                             ProSelecta::Interpreter::kCling)};

  log_trace("  -> selection.fname = {}", analysis->selection.fname);

  std::vector<std::string> ivar_labels;

  auto const &project_functions = xsmeasurement.get_single_projectfuncs();
  auto const &project_prettynames =
      xsmeasurement.get_single_project_prettynames();

  for (size_t vi = 0; vi < ivars.size(); ++vi) {
    auto const &projfs = project_functions[vi];

    std::string prettyname = ivars[vi].name;
    if ((project_prettynames.size() > vi) && (project_prettynames[vi].size())) {
      prettyname = project_prettynames[vi];
    }

    auto units = ivars[vi].units;

    ivar_labels.push_back(prettyname);
    if (units.size()) {
      ivar_labels.back() += fmt::format(" [{}]", ivars[vi].units);
    }

    analysis->projections.push_back(
        {projfs.fname,
         ProSelecta::Get().get_projection_func(projfs.fname,
                                               ProSelecta::Interpreter::kCling),
         prettyname, units});

    log_trace("  -> projection[{}].fname = {}, prettyname = {}", vi,
              analysis->projections.back().fname,
              analysis->projections.back().prettyname);
  }

  analysis->data =
      BinnedValues(Binning::brute_force(to_extents(ivars), ivar_labels), "data",
                   fmt::format("{} [{}]", dvar.prettyname, dvar.units));

  analysis->data.resize();
  analysis->prediction = analysis->data.make_HistFrame();

  for (size_t bin_i = 0; bin_i < dvar.values.size(); ++bin_i) {
    analysis->data.values(bin_i, 0) =
        std::get<double>(dvar.values[bin_i].value);

    if (dvar.values[bin_i].errors.count("total")) {
      analysis->data.errors(bin_i, 0) = dvar.values[bin_i].errors.at("total");
    }
  }

  log_trace("  read data histogram with {} values.",
            analysis->data.values.rows());

  if (xsmeasurement.errors.size()) {
    analysis->error = mat_from_table(xsmeasurement.get_single_errors(),
                                     analysis->data.values.rows());
    log_trace("  * read error matrix");
  } else {

    log_trace("  X no error matrix found");
  }

  if (xsmeasurement.smearings.size()) {
    analysis->smearing = mat_from_table(xsmeasurement.get_single_smearing(),
                                        analysis->data.values.rows());
    log_trace("  * read smearing matrix");
  } else {

    log_trace("  X no smearing matrix found");
  }

  log_trace("  -> targets:");
  for (auto const &wtgt : xsmeasurement.targets[0]) {
    analysis->target.push_back(
        IAnalysis::Target({wtgt->A, wtgt->Z}, wtgt.weight));
    log_trace("    {}", analysis->target.back().to_str());
  }

  auto probe_flux = xsmeasurement.get_single_probe_flux();
  analysis->probe_count.probe_pdg =
      probe_particle_to_pdg(probe_flux.probe_particle);
  analysis->probe_count.spectrum = ProbeFluxToBinnedValuesCount(probe_flux);
  analysis->probe_count.source = probe_flux.source;

  log_trace("  -> flux from: ", probe_flux.source.native());

  if (xsmeasurement.test_statistic != "chi2") {
    throw std::runtime_error(
        "cannot yet process a test test_statistic other than chi2.");
  }

  if (analysis->error.rows()) {
    analysis->likelihood = likelihood::chi2_inv_covariance(
        Eigen::FullPivLU<Eigen::MatrixXd>(analysis->error).inverse());
    log_trace("  -> using chi2 likelihood with pre-inverted error matrix");
  } else {
    analysis->likelihood = likelihood::no_likelihood;
    log_trace("  X no error matrix, using dummy likelihood.");
  }

  auto const &[units, extra_target_scale] =
      get_units_scale(xsmeasurement.cross_section_units,
                      IAnalysis::Target(xsmeasurement.get_simple_target()));

  analysis->xsscale.units = units;
  analysis->xsscale.extra_scale_factor = extra_target_scale;

  analysis->xsscale.divide_by_bin_width =
      xsmeasurement.cross_section_units.count("per_bin_width");

  log_trace("  -> cross section scaling to {}, with an additional target "
            "factor of {}.",
            str_via_ss(units), extra_target_scale);
  log_trace("  -> cross section is differential: {}",
            analysis->xsscale.divide_by_bin_width);

  analysis->finalise =
      finalise::scale_to_cross_section(analysis->xsscale.divide_by_bin_width);

  return analysis;
}

AnalysisPtr HEPDataRecordPlugin::make_MultiDistributionMeasurement(
    HEPData::CrossSectionMeasurement const &xsmeasurement) {

  auto analysis = std::make_shared<SingleFluxAnalysis>();

  size_t nsubm = xsmeasurement.sub_measurements.size();
  size_t nbins = 0;

  std::set<std::filesystem::path> proselecta_sources;
  for (auto const &subm : xsmeasurement.sub_measurements) {
    // load all of the proselecta source files
    proselecta_sources.insert(subm.get_single_selectfunc().source);
    for (auto const &projfs : subm.get_single_projectfuncs()) {
      proselecta_sources.insert(projfs.source);
    }
  }

  for (auto const &src : proselecta_sources) {
    ProSelecta::Get().load_file(src);
  }

  for (size_t subm_i = 0; subm_i < nsubm; ++subm_i) {
    auto const &subm = xsmeasurement.sub_measurements[subm_i];

    auto const &ivars = subm.independent_vars;
    auto const &dvar = subm.dependent_vars[0];

    analysis->selections.push_back(
        {subm.get_single_selectfunc().fname,
         ProSelecta::Get().get_select_func(subm.get_single_selectfunc().fname,
                                           ProSelecta::Interpreter::kCling)});

    std::vector<std::string> ivar_labels;

    auto const &project_functions = subm.get_single_projectfuncs();
    auto const &project_prettynames = subm.get_single_project_prettynames();

    analysis->projections.emplace_back();

    for (size_t vi = 0; vi < ivars.size(); ++vi) {
      auto const &projfs = project_functions[vi];

      std::string prettyname = ivars[vi].name;
      if ((project_prettynames.size() > vi) &&
          (project_prettynames[vi].size())) {
        prettyname = project_prettynames[vi];
      }

      auto units = ivars[vi].units;

      ivar_labels.push_back(prettyname);
      if (units.size()) {
        ivar_labels.back() += fmt::format(" [{}]", ivars[vi].units);
      }

      analysis->projections.back().push_back(
          {projfs.fname,
           ProSelecta::Get().get_projection_func(
               projfs.fname, ProSelecta::Interpreter::kCling),
           prettyname, units});
    }

    analysis->data.push_back(BinnedValues(
        Binning::brute_force(to_extents(ivars), ivar_labels), "data",
        fmt::format("{} [{}]", dvar.prettyname, dvar.units)));

    analysis->data.back().resize();
    nbins += analysis->data.back().values.rows();
    analysis->predictions.push_back(analysis->data.back().make_HistFrame());

    for (size_t bin_i = 0; bin_i < dvar.values.size(); ++bin_i) {
      analysis->data.back().values(bin_i, 0) =
          std::get<double>(dvar.values[bin_i].value);

      if (dvar.values[bin_i].errors.count("total")) {
        analysis->data.back().errors(bin_i, 0) =
            dvar.values[bin_i].errors.at("total");
      }
    }

    if (subm.smearings.size()) {
      if (!analysis->smearings.size()) {
        analysis->smearings.resize(nsubm);
      }
      analysis->smearings[subm_i] = mat_from_table(
          subm.get_single_smearing(), analysis->data.back().values.rows());
    }

    for (auto const &wtgt : subm.targets[0]) {
      if (!analysis->targets.size()) {
        analysis->targets.resize(nsubm);
      }
      analysis->targets[subm_i].push_back(
          IAnalysis::Target({wtgt->A, wtgt->Z}, wtgt.weight));
    }

    if (!analysis->xsscales.size()) {
      analysis->xsscales.resize(nsubm);
    }

    auto const &[units, extra_target_scale] = get_units_scale(
        subm.cross_section_units, IAnalysis::Target(subm.get_simple_target()));

    analysis->xsscales[subm_i].units = units;
    analysis->xsscales[subm_i].extra_scale_factor = extra_target_scale;

    analysis->xsscales[subm_i].divide_by_bin_width =
        subm.cross_section_units.count("per_bin_width");

    analysis->finalise_all.push_back(finalise::scale_to_cross_section(
        analysis->xsscales[subm_i].divide_by_bin_width));

  } // end loop over submeasurements

  // see if we can get away with a single selection function
  bool only_one_selection = true;
  for (size_t si = 1; si < analysis->selections.size(); ++si) {
    if (analysis->selections[0].fname != analysis->selections[si].fname) {
      only_one_selection = false;
      break;
    }
  }
  if (only_one_selection) {
    analysis->selections =
        std::vector<IAnalysis::Selection>{analysis->selections[0]};
  }

  // see if we can get away with a single set of projection functions
  bool only_one_projections = true;
  for (size_t dist_i = 1; dist_i < analysis->projections.size(); ++dist_i) {
    if (analysis->projections[0].size() !=
        analysis->projections[dist_i].size()) {
      only_one_projections = false;
      break;
    }
    for (size_t pi = 1; pi < analysis->projections[dist_i].size(); ++pi) {
      if (analysis->projections[0][pi].fname !=
          analysis->projections[dist_i][pi].fname) {
        only_one_projections = false;
        break;
      }
    }
    if (!only_one_projections) {
      break;
    }
  }
  if (only_one_projections) {
    analysis->projections = std::vector<std::vector<IAnalysis::Projection>>{
        analysis->projections[0]};
  }

  if (xsmeasurement.errors.size()) {
    analysis->error = mat_from_table(xsmeasurement.get_single_errors(), nbins);
  }

  if (xsmeasurement.probe_fluxes.size() == 1) {
    auto probe_flux = xsmeasurement.get_single_probe_flux();
    analysis->probe_count.probe_pdg =
        probe_particle_to_pdg(probe_flux.probe_particle);
    analysis->probe_count.spectrum = ProbeFluxToBinnedValuesCount(probe_flux);
    analysis->probe_count.source = probe_flux.source;
  } else if (xsmeasurement.sub_measurements[0].probe_fluxes.size() == 1) {
    auto probe_flux = xsmeasurement.sub_measurements[0].get_single_probe_flux();
    analysis->probe_count.probe_pdg =
        probe_particle_to_pdg(probe_flux.probe_particle);
    analysis->probe_count.spectrum = ProbeFluxToBinnedValuesCount(probe_flux);
    analysis->probe_count.source = probe_flux.source;
  } else {
    throw InvalidRecordForAnalysisType()
        << "Couldn't find single flux distribution for composite measurement.";
  }

  if (xsmeasurement.test_statistic != "chi2") {
    throw std::runtime_error(
        "cannot yet process a test test_statistic other than chi2.");
  }

  if (analysis->error.rows()) {
    analysis->likelihood = likelihood::chi2_inv_covariance(
        Eigen::FullPivLU<Eigen::MatrixXd>(analysis->error).inverse());
  } else {
    analysis->likelihood = likelihood::no_likelihood;
  }

  return analysis;
}

AnalysisPtr HEPDataRecordPlugin::analysis(YAML::Node const &cfg_in) {

  auto analysisname = cfg_in["analysis"].as<std::string>();

  auto const &xsmeasurement = get_measurement(record, analysisname);

  if (!xsmeasurement.is_composite) {
    return make_SingleDistributionAnalysis(xsmeasurement);
  } else if (xsmeasurement.sub_measurements.size()) {
    return make_MultiDistributionMeasurement(xsmeasurement);
  }

  throw InvalidRecordForAnalysisType()
      << "Can currently only build simple composite measurements that "
         "provide combined errors for multiple distributions.";
}

IRecordPluginPtr HEPDataRecordPlugin::MakeRecord(YAML::Node const &cfg) {
  return std::make_unique<HEPDataRecordPlugin>(cfg);
}

HEPDataRecordPlugin::~HEPDataRecordPlugin() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::HEPDataRecordPlugin::Make, Make);
#endif

} // namespace nuis
