#include "nuis/record/plugins/HEPDataRecordPlugin.h"

#include "nuis/binning/Binning.h"

#include "nuis/record/Utility.h"
#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/SingleAnalysis.h"

#include "nuis/env.h"
#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/HEPData/TableFactory.h"

#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/ranges.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <filesystem>

using namespace nuis;
using namespace ps;

namespace nuis {

NEW_NUISANCE_EXCEPT(InvalidAnalysisForRecord);

// YAML::Node schema() {
//   YAML::Node sc = YAML::Load("{}");
//   sc["type"] = "hepdata";
//   sc["recordref"] = "recordref";
//   return sc;
// }

HEPDataRecordPlugin::HEPDataRecordPlugin(YAML::Node const &cfg) {
  // auto sc = schema();
  // nuis::validate_yaml_map("HEPDataRecordPlugin", sc, cfg);
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
      << "analysis table, nameed " << name
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

    auto su = NuHepMC::GC4::ParseCrossSectionScaleUnits(u);
    if (su != NuHepMC::CrossSection::Units::Scale::CustomType) {
      unit.scale = su;
      continue;
    }

    auto tsu = NuHepMC::GC4::ParseCrossSectionTargetScaleUnits(u);
    if (tsu != NuHepMC::CrossSection::Units::TargetScale::CustomType) {
      unit.tgtscale = tsu;
    } else if ((u == "PerTargetNeutron")) {
      unit.tgtscale =
          NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon;
      extra_target_scale = stgt.A / stgt.N;
    } else if ((u == "PerTargetProton")) {
      unit.tgtscale =
          NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon;
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
  } else if (probe == "nuebar") {
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

AnalysisPtr HEPDataRecordPlugin::analysis(YAML::Node const &cfg_in) {

  auto analysisname = cfg_in["analysis"].as<std::string>();

  auto const &xsmeasurement = get_measurement(record, analysisname);

  if (xsmeasurement.is_composite) {
    throw std::runtime_error(
        "cannot yet handle composite measurements... sorry");
  }

  auto const &ivars = xsmeasurement.independent_vars;
  auto const &dvar = xsmeasurement.dependent_vars[0];

  auto analysis = std::make_shared<SingleAnalysis>();

  // load all of the proselecta source files
  std::set<std::filesystem::path> proselecta_sources;
  proselecta_sources.insert(xsmeasurement.get_single_selectfunc().source);
  for (auto const &projfs : xsmeasurement.get_single_projectfuncs()) {
    proselecta_sources.insert(projfs.source);
  }

  for (auto const &src : proselecta_sources) {
    ProSelecta::Get().load_file(src);
  }

  analysis->select_name = xsmeasurement.get_single_selectfunc().fname;
  analysis->select = ProSelecta::Get().get_select_func(
      xsmeasurement.get_single_selectfunc().fname,
      ProSelecta::Interpreter::kCling);

  for (auto const &projfs : xsmeasurement.get_single_projectfuncs()) {
    analysis->projection_names.push_back(projfs.fname);
    analysis->projections.push_back(ProSelecta::Get().get_projection_func(
        projfs.fname, ProSelecta::Interpreter::kCling));
  }

  std::vector<std::string> ivar_labels;
  auto project_prettynames = xsmeasurement.get_single_project_prettynames();
  for (size_t vi = 0; vi < ivars.size(); ++vi) {
    if (project_prettynames[vi].size()) {
      ivar_labels.push_back(project_prettynames[vi]);
    } else {
      ivar_labels.push_back(ivars[vi].name);
    }
    if (ivars[vi].units.size()) {
      ivar_labels.back() += fmt::format(" [{}]", ivars[vi].units);
    }
  }

  analysis->data = BinnedValues(
      Binning::brute_force(to_extents(xsmeasurement.independent_vars),
                           ivar_labels),
      "data",
      fmt::format("{} [{}]", xsmeasurement.dependent_vars[0].prettyname,
                  xsmeasurement.dependent_vars[0].units));

  analysis->data.resize();
  analysis->prediction = analysis->data.make_HistFrame();

  for (size_t bin_i = 0; bin_i < dvar.values.size(); ++bin_i) {
    analysis->data.values(bin_i, 0) =
        std::get<double>(dvar.values[bin_i].value);

    if (dvar.values[bin_i].errors.count("total")) {
      analysis->data.errors(bin_i, 0) = dvar.values[bin_i].errors.at("total");
    }
  }

  if (xsmeasurement.errors.size()) {
    analysis->error = mat_from_table(xsmeasurement.get_single_errors(),
                                     analysis->data.values.rows());
  }
  if (xsmeasurement.smearings.size()) {
    analysis->smearing = mat_from_table(xsmeasurement.get_single_smearing(),
                                        analysis->data.values.rows());
  }

  for (auto const &wtgt : xsmeasurement.targets[0]) {
    analysis->target.push_back(
        IAnalysis::Target({wtgt->A, wtgt->Z}, wtgt.weight));
  }

  auto probe_flux = xsmeasurement.get_single_probe_flux();
  analysis->probe_particle = probe_particle_to_pdg(probe_flux.probe_particle);
  analysis->probe_flux_count = ProbeFluxToBinnedValuesCount(probe_flux);

  if (xsmeasurement.test_statistic != "chi2") {
    throw std::runtime_error(
        "cannot yet process a test test_statistic other than chi2.");
  }
  analysis->likelihood = likelihood::chi2_covariance(analysis->error);

  auto const &[units, extra_target_scale] =
      get_units_scale(xsmeasurement.cross_section_units,
                      IAnalysis::Target(xsmeasurement.get_simple_target()));

  analysis->xs_units = units;
  analysis->extra_unit_scale = extra_target_scale;

  analysis->per_bin_width =
      xsmeasurement.cross_section_units.count("per_bin_width");

  analysis->finalise =
      finalise::scale_to_cross_section(analysis->per_bin_width);

  return analysis;
}

IRecordPluginPtr HEPDataRecordPlugin::MakeRecord(YAML::Node const &cfg) {
  return std::make_unique<HEPDataRecordPlugin>(cfg);
}

HEPDataRecordPlugin::~HEPDataRecordPlugin() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::HEPDataRecordPlugin::Make, Make);
#endif

} // namespace nuis
