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
#include <map>

#include <Eigen/Dense>

#include "yaml-cpp/yaml.h"
using namespace YAML;

#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"

enum likelihood_form_enum {
     kFullChi2,
     kShapeChi2,
     kNormChi2,
     kShapePlusNormChi2,
     kEvents
};

namespace nuis {
namespace measurement {

enum bands_enum {
     kDataBand = 0,
     kMCBand
};

enum band_selection_enum {
     kAllMC,
     kAll,
     kExceptData
};

struct Band {

     Band(){}

     explicit Band(int nbins,
          std::string lab = "", int bid = 1) {
          band_id = bid;
          label = lab;
          value = Eigen::ArrayXd(nbins);
          count = Eigen::ArrayXi(nbins);
          error = Eigen::ArrayXd(nbins);
     }

     std::string label;
     int band_id;
     Eigen::ArrayXd value;
     Eigen::ArrayXi count;
     Eigen::ArrayXd error;
};


class Projection {
  public:
     Projection();

     explicit Projection(YAML::Node config);

     ~Projection();

     void Fill(int signal,
          const std::vector<double>& indep_vals, double w = 1.0);

     void Scale(double v, int band = kMCBand);

     void Reset(int band = kExceptData);

     // New bin structure scheme
     std::vector<std::string> axis_label;
     std::vector<std::string> sub_label;
     int nbins;

     Eigen::ArrayXi bin_index;
     Eigen::ArrayXi bin_mask;

     Eigen::ArrayXd bin_extent_low;
     Eigen::ArrayXd bin_extent_high;

     std::map<int, Band> bands;

     Eigen::ArrayXd GetBinCenter(){
          return (bin_extent_low + bin_extent_high)/2.0;
     }

     Eigen::ArrayXd GetBinWidth(){
          return (bin_extent_high - bin_extent_low)/2.0;
     }

     Eigen::ArrayXd GetBinHalfWidth(){
          return GetBinWidth()/2.0;
     }

     void AddBand(std::string label,
          int band_id,
          std::vector<double>* value = 0,
          std::vector<double>* error = 0,
          std::vector<int>* count = 0) {
          // Get nbins from bin_info
          if (nbins == 0) {
               std::cerr <<
                    "Trying to add projection bands before bins setup"
                    << std::endl;
          }

          // Build default band
          auto b = Band(nbins, label, band_id);

          // Allow empty errors and count, but not value if provided
          if (!value && (error || count)) {
               std::cerr <<
                    "Warning error or count provided without value!"
                    << std::endl;
          }

          if (value) {
               // We explicitly loop to check matching inputs
               for (size_t i = 0; i < value->size(); i++) {
                    if (value) b.value[i] = value->at(i);
                    if (error) b.error[i] = error->at(i);
                    if (count) b.count[i] = count->at(i);
               }
          }

          // Register new band
          bands[band_id] = b;
     }

     // Create Band (added just because of pythoon default args issue)
     void CreateBand(std::string label,
          int band_id) {
          AddBand(label, band_id, 0, 0, 0);
     }

     Band GetBand(int i) {
          return bands[i];
     }

     Band GetBandFromString(std::string label) {
          if (label == "data") return bands[kDataBand];
          if (label == "mc") return bands[kMCBand];

          for (auto b : bands) {
               if (b.second.label == label) return b.second;
          }

          std::cerr << "Unknown band: " << label << std::endl;
          std::cerr << "Bands Available " << std::endl;
          for (auto b : bands) {
               std::cout << " - " << b.first << " " << b.second.label << std::endl;
          }
          abort();
     }

     // Covariance has to be seperate
     std::string title;
     std::string likelihood_type;
     Eigen::ArrayXd covariance;

     // Structure should be
     //
     // Col0 Col1 Col2 Col3
     // data mc   f2   f3
     //
     
     // Bin data should then be
     // xl xh yl yh zl zh

};

}  // namespace measurement
}  // namespace nuis