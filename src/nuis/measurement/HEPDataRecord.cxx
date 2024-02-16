// Copyright 2016-2021 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

/*******************************************************************************
 *    This file is part of NUISANCE.
 *
 *    NUISANCE is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    NUISANCE is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with NUISANCE.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <map>
#include <filesystem>

#include "nuis/measurement/IRecord.h"

#include "yaml-cpp/yaml.h"
using namespace YAML;

#include "ProSelecta/ProSelecta.h"
#include "HepMC3/GenEvent.h"

#include "nuis/measurement/Projection.h"
#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"
#include "nuis/measurement/HEPDataRecord.h"
#include "spdlog/spdlog.h"


namespace nuis {
namespace measurement {

std::string validate_env() {
  // Require ProSelectra input
  auto PROSELECTA = std::getenv("PROSELECTA_DIR");
  if (!PROSELECTA) {
    spdlog::critical("PROSELECTA_DIR environment variable not defined");
    abort();
  }

  // Require Database Valid
  auto DATABASE = std::getenv("NUISANCEDB");
  if (!DATABASE) {
    spdlog::critical("NUISANCE_DB environment variable not defined");
    abort();
  }

  return std::string(DATABASE);
}

HEPDataRecord::HEPDataRecord(YAML::Node config) {

  std::string DATABASE = validate_env();

  spdlog::info("[INFO]: Loading HEPData Object");
  spdlog::info("[INFO]: --> database: {}", std::string(DATABASE));

  if (!config["release"]){
    spdlog::critical("HEPData node missing release");
    abort();
  }

  // Check if release folder is local
  std::string release = config["release"].as<std::string>();
  std::filesystem::path path_release = release;
  spdlog::info("[INFO]: --> requested release: {}", release);

  if (!std::filesystem::is_directory(path_release.parent_path())) {
    spdlog::info("[INFO]: --> Looking for release in local data folder.");
    release = std::string(DATABASE) + "/neutrino_data/" + release;
    spdlog::info("[INFO]: --> data release: {}", release);
  }

  // Abort if directory not found
  path_release = release;
  if (!std::filesystem::is_directory(path_release.parent_path())) {
    spdlog::critical("HEPData folder is missing");
    abort();
  }
  spdlog::info("[INFO]: --> path: {}", std::string(path_release.parent_path()));


  // Load the YAML Submission file
  std::string filename = release + "/submission.yaml";
  spdlog::info("[INFO]: --> file: {}", filename);


  // Load ProSelectra Snippet
  ProSelecta::Get().AddIncludePath(release);

  std::string analysis = "analysis.cxx";
  if (config["analysis"]) analysis = config["analysis"].as<std::string>();

  if (!ProSelecta::Get().LoadFile(analysis.c_str())) {
    spdlog::critical("[ERROR]: Cling failed interpreting: {}", analysis);
    abort();
  }


  // Tables in hepdata alwayss release _ tablename
  std::string table = config["table"].as<std::string>();
  measurement_name = release + "_" + table;

  // Convert HEPDATA YAML submission to map of documents
  std::vector<YAML::Node> yaml_docs = YAML::LoadAllFromFile(filename);
  std::map<std::string, Document> hepdata_docs;

  for (std::size_t i = 0; i < yaml_docs.size(); i++) {
    Document cur_hepdatadoc = yaml_docs[i].as<Document>();
    hepdata_docs[cur_hepdatadoc.name] = cur_hepdatadoc;
  }

  // Check requested name is inside.
  if (hepdata_docs.find(table) == hepdata_docs.end()) {

    spdlog::critical("[ERROR]: HepData Table not found : {}", table);
    spdlog::critical("[ERROR]: - [ Available Tables ]");
    for (auto const &it : hepdata_docs) {
      spdlog::critical("[ERROR]:  - {}", (it.first));
    }
    abort();

  }

  // Now get individual table info
  std::cout << "[INFO]: Parsing YAML Table for " << table << std::endl;
  measurement_document = hepdata_docs[table];
  std::string datafile = measurement_document.data_file;
  std::string dataname = measurement_document.name;

  std::cout << "Loading YAML Data : " << release + "/" + datafile << std::endl;
  YAML::Node doc = YAML::LoadFile(release + "/" + datafile);
  YAML::Node indep_var_node = doc["independent_variables"];
  for (size_t i = 0; i < indep_var_node.size(); i++) {
    independent_variables.emplace_back(indep_var_node[i].as<Variables>());
  }

  YAML::Node dep_var_node = doc["dependent_variables"];
  for (size_t i = 0; i < dep_var_node.size(); i++) {
    dependent_variables.emplace_back(dep_var_node[i].as<Variables>());
  }

  
  // YAML::Node covariance_doc = YAML::LoadFile(release + "/covar-costhetamupmu_analysisi.yaml");
  // YAML::Node dep_cov_node = covariance_doc["dependent_variables"];
  // for (int i = 0; i < dep_cov_node.size(); i++) {
  //   dependent_covariances.emplace_back(dep_cov_node[i].as<Variables>());
  // }

  // YAML::Node indep_cov_node = covariance_doc["independent_variables"];
  // for (int i = 0; i < indep_cov_node.size(); i++) {
  //   independent_covariances.emplace_back(indep_cov_node[i].as<Variables>());
  // }

  // Get Projections and filters
  std::string filter_symname = dependent_variables[0].qualifiers["Filter"];
  std::cout << "[INFO] : -> Filter : " << filter_symname << std::endl;

  std::vector<std::string> projection_symnames;
  for (size_t i = 0; i < independent_variables.size(); i++) {
    projection_symnames.push_back(
      dependent_variables[0].qualifiers[independent_variables[i].name]);

    std::cout << "[INFO] : Selected Projection : " <<
      independent_variables[i].name << std::endl;
  }

  filter_func = ProSelecta::Get().GetFilterFunction(filter_symname,
    ProSelecta::Interpreter::kCling);

  if (!filter_func) {
    std::cout << "[ERROR]: Cling didn't find a filter function named: "
              << filter_symname << " in the input file. Did you extern \"C\"?"
              << std::endl;
    abort();
  }

  for (auto &proj_sym_name : projection_symnames) {
    auto proj_func = ProSelecta::Get().GetProjectionFunction(proj_sym_name,
      ProSelecta::Interpreter::kCling);

    if (proj_func) {
      proj_funcs.push_back(proj_func);
      proj_funcnames.push_back(proj_sym_name);
    } else {
      std::cerr << "[ERROR]: Cling didn't find a projection function named: "
                << proj_sym_name << " in the input file. Skipping."
                << std::endl;
      abort();
    }
  }
}

std::vector<double> HEPDataRecord::ProjectEvent(const HepMC3::GenEvent& event) {
  std::vector<double> data;
  for (size_t i = 0; i < proj_funcs.size(); ++i) {
    data.push_back(proj_funcs[i](event));
  }
  return data;
}

bool HEPDataRecord::FilterEvent(const HepMC3::GenEvent& event) {
  return filter_func(event);
}

double HEPDataRecord::WeightEvent(const HepMC3::GenEvent& /*event*/) {
  return 1.0;
}

Projection HEPDataRecord::CreateProjection(const std::string label) { 
  // return Projection(measurement_name + "_" + label,
  //   measurement_document,
  //   independent_variables,
  //   dependent_variables,
  //   independent_covariances,
  //   dependent_covariances);
  std::cout << label << std::endl;
    auto project = Projection();
    return project;

    // int nbins  = (dependent_variables)[0].n;

    // std::vector<double> bini;
    // std::vector<double> binj;

    // for (int i = 0; i < in_independent_covariances.size(); i++) {
    //     std::cout << in_independent_covariances[i].name << std::endl;
    //     if (in_independent_covariances[i].name  == "Bini") {
    //         bini = in_independent_covariances[i].values;
    //     }
    //     if (in_independent_covariances[i].name  == "Binj") {
    //         binj = in_independent_covariances[i].values;
    //     }
    // }

    // std::vector<double> covariance_ravel;
    // for (int i = 0; i < in_dependent_covariances.size(); i++) {
    //     std::cout << in_dependent_covariances[i].name << std::endl;
    //     if (in_dependent_covariances[i].name  == "TotalCovariance") {
    //         covariance_ravel = in_dependent_covariances[i].values;
    //     }
    // }

    // std::vector<std::vector<double>> covariance;
    // for (int i = 0; i < bini.size(); i++) {
    //     covariance.push_back( std::vector<double>(bini.size(), 0));
    // }
    // for (int i = 0; i < bini.size(); i++) {
    //     int bi = int(bini[i]);
    //     int bj = int(binj[i]);
    //     covariance[bi][bj] = covariance_ravel[i];
    // }

    // data_value = (dependent_variables)[0].values;
    // if (covariance.size() > 0) {
    //     data_error = std::vector<double>(data_value.size(), 0.0);
    //     for (int i = 0; i < data_value.size(); i++) {
    //         data_error[i] = sqrt(data_covariance[i][i]);
    //     }
    // } else {
    //     data_error = (dependent_variables)[0].errors;
    // }

    // mc_counts = std::vector<uint32_t>(nbins, 0);
    // mc_weights = std::vector<double>(nbins, 0.0);
    // mc_errors = std::vector<double>(nbins, 0.0);

    // for (size_t i = 0; i < nbins; i++) {
    //     bin_index.push_back(i);
    //     bin_mask.push_back(false);

    //     bin_extent_low.push_back(std::vector<double>());
    //     bin_extent_high.push_back(std::vector<double>());
    //     bin_center.push_back(std::vector<double>());
    //     bin_width.push_back(std::vector<double>());

    //     for (int j = 0; j < independent_variables.size(); j++) {
    //         double low  = (independent_variables)[j].low[i];
    //         double high = (independent_variables)[j].high[i];
    //         bin_extent_low[i].push_back(low);
    //         bin_extent_high[i].push_back(high);
    //         bin_center[i].push_back((high+low));
    //         bin_width[i].push_back((high-low));
    //     }
    // }
}

void HEPDataRecord::FinalizeProjection(ProjectionPtr /*h*/, double /*scaling*/) {
}

} 
}



