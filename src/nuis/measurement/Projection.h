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

enum likelihood_form {
     kFullChi2,
     kShapeChi2,
     kNormChi2,
     kShapePlusNormChi2,
     kEvents
};

namespace nuis {
namespace measurement {

class Projection {
  public:
     Projection();

     explicit Projection(YAML::Node config);

     ~Projection();

     void Fill(int signal,
          const std::vector<double>& indep_vals, double w = 1.0);

     void Scale(double v);

     void Reset();

     // New bin structure scheme
     std::string title;
     std::vector<std::string> axis_label;
     std::vector<std::string> sub_label;

     std::string likelihood_type;

     Eigen::ArrayXi bin_index;
     Eigen::ArrayXi bin_mask;

     Eigen::ArrayXd bin_extent_low;
     Eigen::ArrayXd bin_extent_high;

     Eigen::ArrayXd mc_count;
     Eigen::ArrayXd mc_value;
     Eigen::ArrayXd mc_error;

     Eigen::ArrayXd data_value;
     Eigen::ArrayXd data_error;

     Eigen::ArrayXd data_covariance;

};

}  // namespace measurement
}  // namespace nuis