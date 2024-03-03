#include "nuis/record/plugins/IRecordPlugin.h"
#include "nuis/record/Utility.h"
#include "boost/dll/alias.hpp"

#include "nuis/record/Variables.h"
#include "nuis/record/ClearFunctions.h"
#include "nuis/record/WeightFunctions.h"
#include "nuis/record/FinalizeFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

using ClearFunc =
    std::function<void(ComparisonFrame const&)>;

using ProjectFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using WeightFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using SelectFunc =
    std::function<int(HepMC3::GenEvent const &)>;

using FinalizeFunc =
    std::function<void(ComparisonFrame const&)>;

using LikelihoodFunc =
    std::function<double(ComparisonFrame const&)>;

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

    HEPDataRecord(YAML::Node const &cfg) {
        node = cfg;
    }

    TablePtr table(std::string name) {
        std::cout << "Getting HEPRECORD " << name << std::endl;
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

        std::string table = cfg["table"].as<std::string>();


        std::vector<YAML::Node> yaml_docs =
            YAML::LoadAllFromFile(submission_file);


        std::string table_file;
        for (auto const &node : yaml_docs) {
            std::cout << "LOOPING YAML DOCS" << std::endl;
            if (!node["name"]) continue;
            std::cout << "NAME YAML DOCS " << node["name"].as<std::string>() << std::endl;

            if (!node["data_file"]) continue;

            std::cout << "TABLE " << node["name"].as<std::string>() << std::endl;
            if (node["name"].as<std::string>() == table) {
                table_file = node["data_file"].as<std::string>();
            }
        }

        if (table_file.empty()) {
            spdlog::critical("[ERROR]: HepData Table not found : {} {}", table, table_file);
            spdlog::critical("[ERROR]: - [ Available Tables ]");
            for (auto const &node : yaml_docs) {
                if (!node["name"]) continue;
                if (!node["date_file"]) continue;
                spdlog::critical(
                    "[ERROR]:  - {}",
                    node["name"].as<std::string>());
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


        YAML::Node var_node_dep   = table_node["dependent_variables"];

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
            spdlog::critical(
                "[ERROR]: Cling failed interpreting: {}",
                analysis);
            abort();
        }

        std::string filter_name = variables_dep[0].qualifiers["Filter"];
        tab.select = ProSelecta::Get().GetFilterFunction(filter_name,
            ProSelecta::Interpreter::kCling);
        if (!tab.select) {
            std::cout << "[ERROR]: Cling didn't find a filter function named: "
                << filter_name << " in the input file. Did you extern \"C\"?"
                << std::endl;
            abort();
        }


        std::vector<std::string> projection_names;
        for (auto const & iv : variables_indep) {
            projection_names.emplace_back(
                variables_dep[0].qualifiers[iv.name]);
        }

        for (auto const & pn : projection_names) {
            auto pjf = ProSelecta::Get().GetProjectionFunction(pn,
                ProSelecta::Interpreter::kCling);

            if (pjf) {
                tab.projections.emplace_back(pjf);
            } else {
                std::cerr <<
                    "[ERROR]: Cling didn't find a projection function named: "
                    << pn << " in the input file. Skipping."
                    << std::endl;
                abort();
            }
        }

        tab.clear     = nuis::clear::DefaultClear;
        tab.weighting = nuis::weight::DefaultWeight;
        tab.finalize  = nuis::finalize::FATXNormalizedByBinWidth;
        tab.likeihood = nuis::likelihood::Chi2;

        std::vector<nuis::Bins::BinOp> binning;
        for (auto const & iv : variables_indep) {
            std::vector<nuis::Bins::BinningInfo::extent> extents;
            for (size_t bini = 0; bini < iv.low.size(); bini++) {
                extents.push_back({iv.low[bini], iv.high[bini]});
            }
            auto bin_axis = from_extents1D(extents, iv.name);
            binning.emplace_back(bin_axis);
        }

        ComparisonFrame hist(binning[0]);
        tab.blueprint = std::make_shared<ComparisonFrame>(hist);

        std::cout << "Returning Table" << std::endl;
        return std::make_shared<Table>(tab);
    }

    bool good() const { return true; }

    static IRecordPluginPtr Make(YAML::Node const &cfg) {
        std::cout << "Making shared HEPDATA " << std::endl;
        return std::make_shared<HEPDataRecord>(cfg);
    }

    virtual ~HEPDataRecord() {}
};

BOOST_DLL_ALIAS(nuis::HEPDataRecord::Make,
                Make);

} // namespace nuis
