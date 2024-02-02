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

#include "yaml-cpp/yaml.h"
using namespace YAML;

namespace nuis {
namespace measurement {

struct Bin {
    Bin();

    Bin(double icontent,
        std::vector<double> ilow,
        std::vector<double> ihigh);

    explicit Bin(YAML::Node n);

    std::string Summary();
    void Print();

    int Fill(double w = 1);
    int Set(double w, double e = 1);

    bool InRange(const std::vector<double>& v);
    int FillInRange(const std::vector<double>& v, double w = 1);

    void Reset();
    void Erase();

    std::vector<double> extent_low;
    std::vector<double> extent_high;

    double combined_error;
    std::vector<double> seperate_errors;

    int index;
    double content;
    int entries;
};

};  // namespace measurement
};  // namespace nuis
