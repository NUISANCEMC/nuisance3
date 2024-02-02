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
#pragma once
#include <vector>
#include <string>

#include "yaml-cpp/yaml.h"
#include "HepMC3/GenEvent.h"
#include "ProSelecta/ProSelecta.h"

#include "Record.h"
#include "Variables.h"
#include "Document.h"
#include "MeasurementLoader.h"

namespace nuis {
namespace measurement {

class EXFORLoader : public MeasurementLoader {
public:
  EXFORLoader() {}

  explicit EXFORLoader(YAML::Node config);

  virtual ~EXFORLoader() {}

  std::vector<double> ProjectEvent(const HepMC3::GenEvent& event);

  bool FilterEvent(const HepMC3::GenEvent& event);

  double WeightEvent(const HepMC3::GenEvent& event);

  Record CreateRecord(const std::string label="MC");

  bool FillRecordFromEvent(Record& h, const HepMC3::GenEvent& event, const double weight);

  bool FillRecordFromProj(Record& h, const std::vector<double>& val, const double weight);

  void FinalizeRecord(Record& h, double scaling);

  std::string summary();

  void print();

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

  
};

}  // namespace measurement
}  // namespace nuis
