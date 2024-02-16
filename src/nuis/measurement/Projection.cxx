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

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"


#include "nuis/measurement/Projection.h"
#include "nuis/measurement/Variables.h"
#include "nuis/measurement/Document.h"
#include <algorithm>

void str_replace(std::string& in, 
    std::string id1, 
    std::string id2) {

    while( in.find(id1) != std::string::npos) {
        in = in.replace( in.find(id1), 
            id1.length(), 
            id2 );
    }

    return;
}   

namespace nuis{
namespace measurement{

Projection::Projection() {
};

Projection::~Projection() {
}

void Projection::Fill(int signal,
    const std::vector<double>& indep_vals, double w) {

    auto input = Eigen::Map<const Eigen::ArrayXd>\
        (indep_vals.data(), indep_vals.size());

    auto low_result = bin_extent_low.transpose() > input;
    auto high_result = bin_extent_high.transpose() < input;

    // This creates a copy of mc_value, so should avoid for now.
    mc_value = (low_result * high_result).select(mc_value + w, mc_value);
    mc_count = (low_result * high_result).select(mc_value + 1.0, mc_value);

    mc_error = mc_count.sqrt() * mc_value / mc_count;
    return;
}

void Projection::Scale(double s) {
    mc_value.array() *= s;
    mc_error.array() *= s;
}

void Projection::Reset() {
    mc_count.array() = 0.0;
    mc_value.array() = 0.0;
    mc_error.array() = 0.0;
}



}  // namespace measurement
}  // namespace nuis
