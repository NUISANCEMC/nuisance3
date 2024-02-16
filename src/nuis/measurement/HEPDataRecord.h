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
#include <memory>

#include "yaml-cpp/yaml.h"
#include "HepMC3/GenEvent.h"
#include "ProSelecta/ProSelecta.h"

#include "nuis/measurement/Projection.h"
#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"
#include "nuis/measurement/IRecord.h"

namespace nuis {
namespace measurement {

using ProjectionPtr = std::shared_ptr<nuis::measurement::Projection>;

class HEPDataRecord : public IRecord {
public:
  HEPDataRecord() {}

  explicit HEPDataRecord(YAML::Node config);

  virtual ~HEPDataRecord() {}

  std::vector<double> ProjectEvent(const HepMC3::GenEvent& event);

  bool FilterEvent(const HepMC3::GenEvent& event);

  double WeightEvent(const HepMC3::GenEvent& event);

  Projection CreateProjection(const std::string label = "MC");
  
  void FinalizeProjection(ProjectionPtr h, double scaling);
};

}  // namespace measurement
}  // namespace nuis
