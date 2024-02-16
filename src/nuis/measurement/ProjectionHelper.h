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
#include <iostream>
#include <memory>
#include <Eigen/Dense>


#include "yaml-cpp/yaml.h"
using namespace YAML;

#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"

// NUISANCE needs some definition of errors
// as norm need to be handled differently
enum ErrorSource {
     kErrorStatistical = 1,
     kErrorSystematic,
     kErrorNormalisation,
};

namespace nuis {
namespace measurement {

class ProjectionHelper : Projection {
  public:
     ProjectionHelper();

     ~ProjectionHelper();

     std::string Summary();
     void Print();

     void Scale(double v);

     void Reset();

     int GetBin(const std::vector<double>& indep_vals);
     int FillBin(const std::vector<double>& indep_vals, double w = 1.0);
     int FillBin(int i, double w = 1.0);

     int FillBinFromIndex(int i, double w = 1.0) {
          return FillBin(i, w);
     }

     int FillBinFromProjection(const std::vector<double>& indep_vals,
          double w = 1.0) {
          return FillBin(indep_vals, w);
     }

     void ResetBin(int index);
     void ResetBins();
     
     uint32_t GetMCCounts(const uint32_t i);
     double GetMCWeight(const uint32_t i);
     double GetMCError(const uint32_t i);

     double GetTotalMCCounts();
     double GetTotalMCWeight();
     double GetTotalMCValue();

     double GetBinContent(int index);
     double GetBinEntries(int index);

     // The bin error needs a way to select a source
     double GetBinError(int index);

     void SetTally(double v);

     std::vector<double> GetSlice(
          const std::vector<std::vector<double>>& slice,
          const int i);

     // Repeated functions below define a 
     // a more accessible interface for interfacing
     // plots with different drawing tools.
     std::vector<double> GetXCenter();
     std::vector<double> GetYCenter();
     std::vector<double> GetZCenter();

     std::vector<double> GetXEdge(bool low = true);
     std::vector<double> GetYEdge(bool low = true);
     std::vector<double> GetZEdge(bool low = true);

     std::vector<double> GetXWidth();
     std::vector<double> GetYWidth();
     std::vector<double> GetZWidth();

     std::vector<double> GetXErr();
     std::vector<double> GetYErr();
     std::vector<double> GetZErr();

     std::vector<double> GetMC();
     std::vector<double> GetMCErr();
};



}  // namespace measurement
}  // namespace nuis