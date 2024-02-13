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

#include "yaml-cpp/yaml.h"

#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"

#define errIndexStat 1
#define errIndexSyst 2
#define errIndexNorm 3

using namespace YAML;

namespace nuis {
namespace measurement {

class Record {
  public:
     Record();

     Record(YAML::Node config);

     Record(std::string iname,
          const Document& in_document,
          const std::vector<Variables>& in_independent_variables,
          const std::vector<Variables>& in_dependent_variables,
          const std::vector<Variables>& in_dependent_covariances,
          const std::vector<Variables>& in_independent_covariances);

     ~Record();

     std::string Summary();
     void Print();

     void Scale(double v);

     // int GetTotalEntries();
     // double GetTotalWeight();
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



     double Metadata;

     // New bin structure scheme
     std::string label;
     std::string name;
     std::string title;

     // Document document;
     // std::vector<Variables> dependent_variables;
     // std::vector<Variables> independent_variables;

     std::vector<std::vector<double>> bin_extent_low;
     std::vector<std::vector<double>> bin_extent_high;
     std::vector<std::vector<double>> bin_center;
     std::vector<std::vector<double>> bin_width;

     std::vector<int>  bin_index;
     std::vector<bool> bin_mask;

     std::vector<double> data_value;
     std::vector<double> data_error;
     std::vector<uint32_t> mc_counts;
     std::vector<double>   mc_weights;
     std::vector<double>   mc_errors;


     std::vector<std::vector<double>> data_covariance;

     int total_mc_counts;
     double total_mc_weights;
     int total_mc_tally;

};



}  // namespace measurement
}  // namespace nuis