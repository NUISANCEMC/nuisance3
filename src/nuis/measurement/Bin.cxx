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
#include "nuis/measurement/Bin.h"

namespace nuis {
namespace measurement {

Bin::Bin(){
}

Bin::Bin(double icontent, 
    std::vector<double> ilow,
    std::vector<double> ihigh){
}
    
Bin::Bin(YAML::Node n){
}

bool Bin::InRange(const std::vector<double>& v){
    
    for (size_t j = 0; j < v.size(); j++){

        if (j >= extent_low.size()) break;
        if (j >= extent_high.size()) break;
        
        if (v[j] < extent_low[j] ||
            v[j] > extent_high[j]) {
                return false;
        }
    }
    return true;
}

std::string Bin::Summary(){
    std::string s = "HEPData.Bin";
    return s;
}

void Bin::Print(){
    std::cout << Summary() << std::endl;
}

int Bin::Fill(double w){
    content += w;
    entries += 1;
    std::cout << "BIN " << content << std::endl;
    return index;
}

int Bin::Set(double w, double e){
    content = w;
    entries = e;
    return index;
}

int Bin::FillInRange(const std::vector<double>& v, double w){
    if (!InRange(v)) return 0;
    Fill(w);
    return index;
}

void Bin::Reset(){
    content = 0;
    entries = 0;
    combined_error = 0;
    seperate_errors.clear();
}

void Bin::Erase(){
    extent_low.clear();
    extent_high.clear();
    Reset();
}

} // - namespace measurement
} // - namespace nuis
