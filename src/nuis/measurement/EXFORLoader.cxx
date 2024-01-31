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

#include "nuis/measurement/Measurement.h"

#include "yaml-cpp/yaml.h"
using namespace YAML;

#include "ProSelecta/ProSelecta.h"
#include "HepMC3/GenEvent.h"

#include "nuis/measurement/Record.h"
#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"
#include "nuis/measurement/HEPDataLoader.h"

namespace nuis {
namespace measurement {

EXFORLoader::EXFORLoader(YAML::Node config) {
  std::cout << "[INFO]: Creating EXFOR Loader" << std::endl;

  // isotope=C12
  // interaction=2
  // ceam=proton
  // min_energy
  // max_energy
  // analysis=
  // [tobeadded] reference=""

  beam = "proton";
  if (config["beam"]) config["beam"].as<std::string>();

  interaction = "total";
  if (config["total"]) interaction = config["total"].as<std::string>();

  isotope = "12C"
  if (config["isotope"]) isotope = config["isotope"].as<std::string>();

  min_energy = -1;
  max_energy = -1;
  analysis = "analysis.cxx";
  reference = "";

  // Now form NUISANCE objects

  // - First check anaysis loads
  release = beam + "_data/EXFOR/";
  std::filesystem::path path_release = release;
  if (!std::filesystem::is_directory(path_release.parent_path())) {
    release = std::string(getenv("NUISANCEDB")) + "/" + release;
  }
  
  ProSelecta::Get().AddIncludePath(release);
  ProSelecta::Get().AddIncludePath(std::string(getenv("PROSELECTA_DIR")));

  if (!ProSelecta::Get().LoadFile(analysis.c_str())) {
    std::cout << "[ERROR]: Cling failed interpreting: "\
      << analysis << std::endl;
    throw(505);
  }

  // Then parse the actual data file to fill in the measurement records
  ParseEXFORDataFile("reactions_isotope_" + isotope + "_nuisance.csv");
  
  measurement_name = "exfor_" + isotope + "_" + beam;

  // Now get individual table info
  std::cout << "[INFO]: Building YAML Table for " << table << std::endl;
  measurement_document = YAML::Node();
  std::string datafile = measurement_document.data_file;
  std::string dataname = measurement_document.name;

  YAML::Node indep_var_node = doc["independent_variables"];
  for (int i = 0; i < indep_var_node.size(); i++) {
    independent_variables.emplace_back(indep_var_node[i].as<Variables>());
  }

  YAML::Node dep_var_node = doc["dependent_variables"];
  for (int i = 0; i < dep_var_node.size(); i++) {
    dependent_variables.emplace_back(dep_var_node[i].as<Variables>());
  }

  // Get Projections and filters
  std::string filter_symname = dependent_variables[0].qualifiers["Filter"];
  std::cout << "[INFO] : -> Filter : " << filter_symname << std::endl;

  std::vector<std::string> projection_symnames;
  for (int i = 0; i < independent_variables.size(); i++) {
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
    throw(505);
  }

  for (auto &proj_sym_name : projection_symnames) {
    auto proj_func = ProSelecta::Get().GetProjectionFunction(proj_sym_name,
      ProSelecta::Interpreter::kCling);

    if (proj_func) {
      proj_funcs.push_back(proj_func);
      proj_funcnames.push_back(proj_sym_name);
    } else {
      std::cout << "[ERROR]: Cling didn't find a projection function named: "
                << proj_sym_name << " in the input file. Skipping."
                << std::endl;
      throw(505);
    }
  }
}

std::vector<double> EXFORLoader::ProjectEvent(const HepMC3::GenEvent& event) {
  std::vector<double> data;
  for (size_t i = 0; i < proj_funcs.size(); ++i) {
    data.push_back(proj_funcs[i](event));
  }
  return data;
};

bool EXFORLoader::FilterEvent(const HepMC3::GenEvent& event) {
  return filter_func(event);
};

double EXFORLoader::WeightEvent(const HepMC3::GenEvent& event) {
  return 1.0;
}

Record EXFORLoader::CreateRecord(const std::string label) { 
  return Record(measurement_name,
    measurement_document,
    independent_variables,
    dependent_variables);
};

bool EXFORLoader::FillRecordFromEvent(Record& h,
  const HepMC3::GenEvent& e, const double weight) {
  h.FillTally();
  if (!FilterEvent(e)) return false;
  h.FillBin(ProjectEvent(e), weight * WeightEvent(e));
  return true;
}

bool EXFORLoader::FillRecordFromProj(Record& h, 
  const std::vector<double>& v, const double weight) {
  h.FillTally();  // Hack so that Records record total passed through.
  h.FillBin(v, weight);
  return true;
}

void EXFORLoader::FinalizeRecord(Record& h, double scaling) {
  h.Scale(scaling / h.GetTally());
}

std::string EXFORLoader::summary() {
  return "";
}

void EXFORLoader::print() {
  return;
}

}
}



