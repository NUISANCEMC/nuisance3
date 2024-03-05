#include "nuis/histframe/Binning.h"
#include "nuis/record/Utility.h"
#include "nuis/record/plugins/IRecordPlugin.h"

#include "boost/dll/alias.hpp"

#include "nuis/record/plugins/HEPDataVariables.h"

#include "nuis/record/ClearFunctions.h"
#include "nuis/record/FinalizeFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

using namespace nuis;

// The functions for binning are pretty opaque to new users. I think comments
// need to label these as the efficient implementations and some simple
// examples. For now to make sure things are working I'm adding a HEPData brute
// force search one so we can be sure its working.
nuis::Binning from_hepdata_extents(std::vector<Variables> &axes) {

  nuis::Binning bin_info;
  for (auto const &ax : axes) {
    bin_info.axis_labels.push_back(ax.name);
  }

  // Make the containers as its transposed I think
  for (size_t j = 0; j < axes[0].n; j++) {
    bin_info.bins.emplace_back();
  }

  // HEPData format is in vector<x>, vector<y>
  for (size_t i = 0; i < axes.size(); i++) {
    for (size_t j = 0; j < axes[i].n; j++) {
      Binning::SingleExtent vp = {axes[i].low[j], axes[i].high[j]};
      bin_info.bins[j].emplace_back(vp);
    }
  }

  bin_info.find_bin = [=](std::vector<double> const &x) -> Binning::Index {
    for (size_t i = 0; i < bin_info.bins.size(); i++) {
      const std::vector<Binning::SingleExtent> &bin_info_slice =
          bin_info.bins[i];

      bool goodbin = true;
      for (size_t j = 0; j < bin_info_slice.size(); j++) {
        if (x[j] < bin_info_slice[j].min) {
          goodbin = false;
          break;
        }
        if (x[j] >= bin_info_slice[j].max) {
          goodbin = false;
          break;
        }
      }
      if (!goodbin)
        continue;
      return i;
    }
    return Binning::npos;
  };
  return bin_info;
}

namespace nuis {

using ClearFunc = std::function<void(ComparisonFrame &)>;

using ProjectFunc = std::function<double(HepMC3::GenEvent const &)>;

using WeightFunc = std::function<double(HepMC3::GenEvent const &)>;

using SelectFunc = std::function<int(HepMC3::GenEvent const &)>;

// Don't love that this needs all at once
using FinalizeFunc = std::function<void(ComparisonFrame &, const double)>;

using LikelihoodFunc = std::function<double(ComparisonFrame const &)>;

class HEPDataRecord : public IRecordPlugin {
public:
  std::string db_path;

  YAML::Node schema() {
    YAML::Node sc = YAML::Load("{}");
    sc["type"] = "hepdata";
    sc["release"] = "release_name";
    sc["table"] = "table_name";
    return sc;
  }

  HEPDataRecord(YAML::Node const &cfg) { node = cfg; }

  TablePtr table(std::string table) {
    YAML::Node cfg = node;

    db_path = nuis::database();

    auto sc = schema();
    nuis::validate_yaml_map("HEPDATARecord", sc, cfg);

    std::string release = cfg["release"].as<std::string>();

    std::string path_release = db_path + "/neutrino_data/" + release;
    if (!std::filesystem::is_directory(path_release)) {
      spdlog::critical("HEPData folder is missing: {}", path_release);
      abort();
    }

    std::string submission_file = path_release + "/submission.yaml";
    // if (!std::filesystem::is_file(submission_file)) {
    // spdlog::critical(
    // "HEPData folder is missing submissino.yaml : {}",
    // pasubmission_fileth_release);
    // abort();
    // }

    // std::string table = cfg["table"].as<std::string>();

    std::vector<YAML::Node> yaml_docs = YAML::LoadAllFromFile(submission_file);

    std::string table_file;
    for (auto const &node : yaml_docs) {
      if (!node["name"])
        continue;
      if (!node["data_file"])
        continue;

      if (node["name"].as<std::string>() == table) {
        table_file = node["data_file"].as<std::string>();
      }
    }

    if (table_file.empty()) {
      spdlog::critical("[ERROR]: HepData Table not found : {} {}", table,
                       table_file);
      spdlog::critical("[ERROR]: - [ Available Tables ]");
      for (auto const &node : yaml_docs) {
        if (!node["name"])
          continue;
        if (!node["date_file"])
          continue;
        spdlog::critical("[ERROR]:  - {}", node["name"].as<std::string>());
      }
      abort();
    }

    YAML::Node table_node = YAML::LoadFile(path_release + "/" + table_file);
    auto tab = Table();

    YAML::Node var_node_indep = table_node["independent_variables"];

    std::vector<Variables> variables_indep;
    for (auto const iv : var_node_indep) {
      variables_indep.emplace_back(iv.as<Variables>());
    }

    if (variables_indep.empty()) {
      spdlog::critical("[ERROR]: HepData Independent Variables len == 0");
    }

    YAML::Node var_node_dep = table_node["dependent_variables"];

    std::vector<Variables> variables_dep;
    for (auto const iv : var_node_dep) {
      variables_dep.emplace_back(iv.as<Variables>());
    }

    if (variables_dep.empty()) {
      spdlog::critical("[ERROR]: HepData dependent Variables len == 0");
    }

    // This seems to be ProSelecta bug feature, analysis.cxx works
    // but /path/analysis.cxx does not.
    ProSelecta::Get().AddIncludePath(path_release.c_str());
    std::string analysis = path_release + "analysis.cxx";
    if (cfg["analysis"])
      analysis = cfg["analysis"].as<std::string>();

    if (!ProSelecta::Get().LoadFile("analysis.cxx")) {
      spdlog::critical("[ERROR]: Cling failed interpreting: {}", analysis);
      abort();
    }

    std::string filter_name = variables_dep[0].qualifiers["Filter"];
    tab.select = ProSelecta::Get().GetFilterFunction(
        filter_name, ProSelecta::Interpreter::kCling);
    if (!tab.select) {
      std::cout << "[ERROR]: Cling didn't find a filter function named: "
                << filter_name << " in the input file. Did you extern \"C\"?"
                << std::endl;
      abort();
    }

    std::vector<std::string> projection_names;
    for (auto const &iv : variables_indep) {
      projection_names.emplace_back(variables_dep[0].qualifiers[iv.name]);
    }

    for (auto const &pn : projection_names) {
      auto pjf = ProSelecta::Get().GetProjectionFunction(
          pn, ProSelecta::Interpreter::kCling);

      if (pjf) {
        tab.projections.emplace_back(pjf);
      } else {
        std::cerr << "[ERROR]: Cling didn't find a projection function named: "
                  << pn << " in the input file. Skipping." << std::endl;
        abort();
      }
    }

    tab.clear = nuis::clear::DefaultClear;

    tab.project = [tab](HepMC3::GenEvent const &ev) {
      std::vector<double> v;
      for (auto const &p : tab.projections) {
        v.emplace_back(p(ev));
      }
      return v;
    };

    tab.weight = nuis::weight::DefaultWeight;
    tab.finalize = nuis::finalize::EventRateScaleToData;
    tab.likeihood = nuis::likelihood::Chi2;

    nuis::Binning binning = from_hepdata_extents(variables_indep);

    ComparisonFrame hist(binning);

    hist.data.contents.col(0) = Eigen::Map<Eigen::VectorXd>(
        variables_dep[0].values.data(), variables_dep[0].values.size());

    hist.data.variance.col(0) = Eigen::Map<Eigen::VectorXd>(
        variables_dep[0].errors.data(), variables_dep[0].errors.size());

    tab.blueprint = std::make_shared<ComparisonFrame>(hist);

    return std::make_shared<Table>(tab);
  }

  bool good() const { return true; }

  static IRecordPluginPtr Make(YAML::Node const &cfg) {
    return std::make_shared<HEPDataRecord>(cfg);
  }

  virtual ~HEPDataRecord() {}
};

BOOST_DLL_ALIAS(nuis::HEPDataRecord::Make, Make);

} // namespace nuis
