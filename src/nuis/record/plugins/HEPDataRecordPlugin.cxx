#include "nuis/record/plugins/HEPDataRecordPlugin.h"

#include "nuis/binning/Binning.h"

#include "nuis/record/Utility.h"
#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/FinalizeFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/SimpleAnalysis.h"
#include "nuis/record/WeightFunctions.h"

#include "nuis/env.h"
#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/HEPData/TableFactory.hxx"

#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/ranges.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <filesystem>

using namespace nuis;
using namespace ps;

namespace nuis {

NEW_NUISANCE_EXCEPT(InvalidTableForRecord);

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

  auto recordref =
      HEPData::ResourceReference(cfg["recordref"].as<std::string>());

  log_debug("Reading hepdata-format record from reference: {}",
            recordref.str());

  record = HEPData::make_Record(recordref, nuis::env::NUISANCEDB());
}

auto get_measurement(HEPData::Record const &rec, std::string const &name) {
  for (auto const &xsm : rec.measurements) {
    if (name == xsm.name) {
      return xsm;
    }
  }

  std::vector<std::string> table_names;
  for (auto const &xsm : rec.measurements) {
    table_names.push_back(xsm.name);
  }

  log_critical(
      "requested analysis table, named \"{}\", does not exist on record {}. Valid "
      "tables: {}",
      name, rec.record_ref.str(), table_names);
  throw InvalidTableForRecord()
      << "analysis table, nameed " << name
      << ", does not exist on record: " << rec.record_ref.str();
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

struct target {
  target(long pid)
      : A{NuHepMC::CrossSection::Units::NuclearPDGToZ(pid)},
        Z{NuHepMC::CrossSection::Units::NuclearPDGToA(pid)} {}
  target(int a, int z) : A{a}, Z{z} {}
  int A, Z;
  int N() { return A - Z; }
};

target get_target(std::string const &tgtstr) {
  try {
    return target(std::stol(tgtstr));
  } catch (std::invalid_argument &ia) {
  }

  static std::map<std::string, target> const known_targets = {
      {"C", target{12, 6}},
      {"CH", target{13, 7}},
      {"O", target{16, 8}},
      {"H2O", target{18, 10}},
      {"Ar", target{18, 40}}};

  if (!known_targets.count(tgtstr)) {
    throw std::runtime_error(
        fmt::format("unknown target specifier: {}", tgtstr));
  }

  return known_targets.at(tgtstr);
}

auto get_units_scale(std::set<std::string> const &units_bits,
                     std::string const &tgtstr) {
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
      auto tgt = get_target(tgtstr);
      extra_target_scale = tgt.A / tgt.N();
    } else if ((u == "PerTargetProton")) {
      unit.tgtscale =
          NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon;
      auto tgt = get_target(tgtstr);
      extra_target_scale = tgt.A / tgt.Z;
    }
  }

  return std::make_pair(unit, extra_target_scale);
}

AnalysisPtr HEPDataRecordPlugin::analysis(YAML::Node const &cfg_in) {

  auto analysisname = cfg_in["analysis"].as<std::string>();

  auto const &xsmeasurement = get_measurement(record, analysisname);

  if (xsmeasurement.is_composite) {
    throw std::runtime_error(
        "cannot yet handle composite measurements... sorry");
  }

  auto analysis = std::make_shared<SimpleAnalysis>();

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

  analysis->data = BinnedValues(
      Binning::brute_force(to_extents(xsmeasurement.independent_vars),
                           analysis->projection_names),
      "data");

  analysis->data.resize();
  analysis->prediction = analysis->data.make_HistFrame();

  for (size_t bin_i = 0; bin_i < xsmeasurement.dependent_vars[0].values.size();
       ++bin_i) {
    analysis->data.values(bin_i, 0) =
        std::get<double>(xsmeasurement.dependent_vars[0].values[bin_i].value);
  }

  if (xsmeasurement.test_statistic != "chi2") {
    throw std::runtime_error(
        "cannot yet process a test test_statistic other than chi2.");
  }
  // analysis->likelihood = likelihood::Chi2;

  auto const &[units, extra_target_scale] = get_units_scale(
      xsmeasurement.cross_section_units, xsmeasurement.get_single_target());

  analysis->xs_units = units;
  analysis->extra_unit_scale = extra_target_scale;

  bool is_per_width = xsmeasurement.cross_section_units.count("per_bin_width");

  analysis->finalise = is_per_width ? finalise::FATXNormalizedByBinWidth
                                    : finalise::FATXNormalized;

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
