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
    nbins = 0;
};

Projection::~Projection() {
}

void Projection::Fill(int signal,
    const std::vector<double>& indep_vals, double w) {

    if (signal == 0) return; 

    // auto input = Eigen::Map<const Eigen::ArrayXd>
        // (indep_vals.data(), indep_vals.size());

    // auto input = indep_vals[0];
    // auto islow = (bin_extent_low.array() < input);
    // auto ishigh = (bin_extent_high.array() > input);

    for (size_t irow = 0; irow < bin_extent_low.rows(); irow++) {
        bool valid = true;
        for (size_t j = 0; j < indep_vals.size(); j++) {
            // std::cout << "COMPARING : " << indep_vals[j] << " vs " << bin_extent_low(irow, j) << " " << bin_extent_high(irow, j) << std::endl;
            if (indep_vals[j] < bin_extent_low(irow, j)) {
                valid = false;
                break;
            }
            if (indep_vals[j] > bin_extent_high(irow, j)) {
                valid = false;
                break;
            }
        }

        if (valid) {
            // std::cout << "FOUND and filling " << signal << " " << w << std::endl;
            bands[signal].value(irow) += w;
            bands[signal].count(irow) += 1;
        }

    }
    


    // std::cout << "COMPARISON" << std::endl;
    // std::cout << input << std::endl;
    // std::cout << "-low" << std::endl;
    // std::cout << bin_extent_low << std::endl;
    // std::cout << "-high" << std::endl;
    // std::cout << bin_extent_high << std::endl;
    // std::cout << "-islow" << std::endl;
    // std::cout << islow << std::endl;

    // std::cout << "DONE COMPARISON" << std::endl;

    // auto value_add = (bin_extent_low > input).select((bin_extent_high < input).select(w, 0), 0);
    
    // auto count_add = (bin_extent_low > input).select((bin_extent_high < input).select(1, 0), 0);

    // // auto high_result = bin_extent_high < input;
    // // auto comb_result = low_result;
    // // comb_result *= high_result;

    // std::cout << "COMPARISON" << std::endl;
    // std::cout << value_add << std::endl;


    // // if (value.find(signal) == value.end()) {
    // //     value[signal] = value[0];
    // //     value[signal] = count[0];
    // // }

    // auto b = bands[signal];

    // std::cout << "END OF BOOOL COMP" << std::endl;
    // // Not the most efficient select...
    // // b.value = (comb_result).select(b.value + w,   b.value);
    // // b.count = (comb_result).select(b.count + 1,   b.count);

    // std::cout << "VALUE:" << b.value;
    // bands[signal].value += (islow*ishigh).cast<double>()*w;
    // bands[signal].count += (islow*ishigh).cast<int>();
    // std::cout << "VALUE AFTTER :" << b.value;

    return;
}

void Projection::Scale(double s, int band) {
    auto b = bands[band];
    b.value.array() *= s;
    b.error.array() *= s;
}

void Projection::Reset(int /*band*/) {
    // auto b = bands[band];
    // if (band == kExceptData) {
    //     for (int i = 0; i < count.size(); i++) {
    //         b.value.array() = 0.0;
    //         b.count.array() = 0.0;
    //         b.error.array() = 0.0;
    //     }
    // }
}

}  // namespace measurement
}  // namespace nuis
