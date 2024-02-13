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

#include "nuis/measurement/Record.h"
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

Record::Record()
: total_mc_tally(0) {
};

Record::Record(YAML::Node config) {
    if (config["bin_extent_low"])
        bin_extent_low = config["bin_extent_low"]\
            .as<std::vector<std::vector<double> > >();

    if (config["bin_extent_high"])
        bin_extent_high = config["bin_extent_high"]\
            .as<std::vector<std::vector<double> > >();

    if (config["bin_index"])
        bin_index = config["bin_index"].as<std::vector<int> >();

    if (config["bin_mask"])
        bin_mask = config["bin_mask"].as<std::vector<bool> >();

    if (config["data_value"])
        data_value = config["data_value"].as<std::vector<double> >();

    if (config["data_error"])
        data_error = config["data_error"].as<std::vector<double> >();

    if (config["mc_counts"])
        mc_counts = config["mc_counts"].as<std::vector<uint32_t> >();

    if (config["mc_weights"])
        mc_weights = config["mc_weights"].as<std::vector<double> >();

    if (config["mc_errors"])
        mc_errors = config["mc_errors"].as<std::vector<double> >();
}

Record::Record(std::string iname,
    const Document& in_document,
    const std::vector<Variables>& in_independent_variables,
    const std::vector<Variables>& in_dependent_variables,
    const std::vector<Variables>& in_independent_covariances,
    const std::vector<Variables>& in_dependent_covariances) :

    total_mc_tally(0) {
    // Make output ready name
    str_replace(iname, "..", "");
    str_replace(iname, "/_", "-");
    str_replace(iname, "/", "-");
    str_replace(iname, "@", "");

    name = "@" + iname + ":";
    label = "v1";
    Document document = in_document;
    std::vector<Variables> dependent_variables   = in_dependent_variables;
    std::vector<Variables> independent_variables = in_independent_variables;

    // Must be only one indepenendent at the moment
    if (dependent_variables.size() > 1) {
        std::cout << "[ERR]"\
            << ": Can only handle 1 dependent variable right now : "
            <<  dependent_variables.size() << std::endl;
        throw;
    }

    int nbins  = (dependent_variables)[0].n;

    std::vector<double> bini;
    std::vector<double> binj;

    for (int i = 0; i < in_independent_covariances.size(); i++) {
        std::cout << in_independent_covariances[i].name << std::endl;
        if (in_independent_covariances[i].name  == "Bini") {
            bini = in_independent_covariances[i].values;
        }
        if (in_independent_covariances[i].name  == "Binj") {
            binj = in_independent_covariances[i].values;
        }
    }

    std::vector<double> covariance_ravel;
    for (int i = 0; i < in_dependent_covariances.size(); i++) {
        std::cout << in_dependent_covariances[i].name << std::endl;
        if (in_dependent_covariances[i].name  == "TotalCovariance") {
            covariance_ravel = in_dependent_covariances[i].values;
        }
    }
    
    std::vector<std::vector<double>> covariance;
    for (int i = 0; i < bini.size(); i++) {
        data_covariance.push_back( std::vector<double>(bini.size(), 0));
    }
    for (int i = 0; i < bini.size(); i++) {
        int bi = int(bini[i]);
        int bj = int(binj[i]);
        data_covariance[bi][bj] = covariance_ravel[i];
    }

    data_value = (dependent_variables)[0].values;
    if (covariance.size() > 0) {
        data_error = std::vector<double>(data_value.size(), 0.0);
        for (int i = 0; i < data_value.size(); i++) {
            data_error[i] = sqrt(data_covariance[i][i]);
        }
    } else {
        data_error = (dependent_variables)[0].errors;
    }

    total_mc_counts = 0;
    total_mc_weights = 0;
    total_mc_tally = 0;

    mc_counts = std::vector<uint32_t>(nbins, 0);
    mc_weights = std::vector<double>(nbins, 0.0);
    mc_errors = std::vector<double>(nbins, 0.0);

    for (size_t i = 0; i < nbins; i++) {
        bin_index.push_back(i);
        bin_mask.push_back(false);

        bin_extent_low.push_back(std::vector<double>());
        bin_extent_high.push_back(std::vector<double>());
        bin_center.push_back(std::vector<double>());
        bin_width.push_back(std::vector<double>());

        for (int j = 0; j < independent_variables.size(); j++) {
            double low  = (independent_variables)[j].low[i];
            double high = (independent_variables)[j].high[i];
            bin_extent_low[i].push_back(low);
            bin_extent_high[i].push_back(high);
            bin_center[i].push_back((high+low));
            bin_width[i].push_back((high-low));
        }
    }
}

Record::~Record() {
}

void Record::Reset() {
    ResetBins();
}

void Record::ResetBins() {
    for (size_t i = 0; i < mc_counts.size(); i++) {
        mc_counts[i] = 0;
        mc_weights[i] = 0;
        mc_errors[i] = 0;
    }
}

int Record::GetBin(const std::vector<double>& invals) {
    for (int i = 0; i < mc_counts.size(); i++) {
        bool foundbin = true;

        for (size_t j = 0; j < invals.size(); j++) {
            if (invals[j] < bin_extent_low[i][j]) {
                foundbin = false;
                break;
            }
            if (invals[j] > bin_extent_high[i][j]) {
                foundbin = false;
                break;
            }
        }
        if (foundbin) return i;
    }

    return -1;
}

int Record::FillBin(int i, double w) {
    if (i < 0) return -1;

    mc_counts[i] += 1;
    mc_weights[i] += w;

    mc_errors[i] = sqrt(mc_counts[i]) / mc_counts[i];
    mc_errors[i] *= mc_weights[i];

    total_mc_counts = 1;
    total_mc_weights = w;

    return i;
}


int Record::FillBin(const std::vector<double>& indep_vals, double w) {
    int i = GetBin(indep_vals);
    return FillBin(i, w);
}

uint32_t Record::GetMCCounts(const uint32_t i) {
    return mc_counts[i];
}

double Record::GetMCWeight(const uint32_t i) {
    return mc_weights[i];
}

double Record::GetMCError(const uint32_t i) {
    return mc_errors[i];
}

double Record::GetTotalMCCounts() {
    int tot = 0;
    for (auto const& b : mc_counts) {
        tot += b;
    }
    return tot;
}

double Record::GetTotalMCWeight() {
    int tot = 0;
    for (auto const& b : mc_weights) {
        tot += b;
    }
    return tot;
}

double Record::GetTotalMCValue() {
    return GetTotalMCWeight();
}

std::string Record::Summary() {
    return "";
}

void Record::Scale(double s) {
    for (int i = 0; i < mc_counts.size(); i++) {
        mc_counts[i] *= s;
    }
}

void Record::Print() {
    std::cout << Summary() << std::endl;
}

double Record::GetBinContent(int index) {
    return GetMCWeight(index);
}

double Record::GetBinEntries(int index) {
    return GetMCCounts(index);
}

double Record::GetBinError(int index) {
    return GetMCError(index);
}

std::vector<double> Record::GetSlice(
    const std::vector<std::vector<double>>& slice,
    int i) {

    std::vector<double> v1;

    std::transform(bin_center.begin(),
        bin_center.end(),
        std::back_inserter(v1),
        [&i](const std::vector<double>& value) {
        return value[i];
    });

    return v1;
}

std::vector<double> Record::GetXCenter() {
    return GetSlice(bin_center, 0);
}

std::vector<double> Record::GetYCenter() {
    return GetSlice(bin_center, 1);
}

std::vector<double> Record::GetZCenter() {
    return GetSlice(bin_center, 2);
}

std::vector<double> Record::GetXEdge(bool low) {
    return GetSlice(low ? bin_extent_low : bin_extent_high, 0);
}

std::vector<double> Record::GetYEdge(bool low) {
    return GetSlice(low ? bin_extent_low : bin_extent_high, 1);
}

std::vector<double> Record::GetZEdge(bool low) {
    return GetSlice(low ? bin_extent_low : bin_extent_high, 2);
}

std::vector<double> Record::GetXWidth() {
    return GetSlice(bin_width, 0);
}

std::vector<double> Record::GetYWidth() {
    return GetSlice(bin_width, 1);
}

std::vector<double> Record::GetZWidth() {
    return GetSlice(bin_width, 2);
}

std::vector<double> Record::GetMC() {
    return mc_weights;
}
std::vector<double> Record::GetMCErr() {
    return mc_errors;
}

std::vector<double> Record::GetXErr() {
    std::vector<double> v = GetSlice(bin_width, 0);
    for_each(v.begin(), v.end(), [](double &c){ c /= 2.0; });
    return v;
}

std::vector<double> Record::GetYErr() {
    std::vector<double> v = GetSlice(bin_width, 1);
    for_each(v.begin(), v.end(), [](double &c){ c /= 2.0; });
    return v;
}

std::vector<double> Record::GetZErr() {
    std::vector<double> v = GetSlice(bin_width, 2);
    for_each(v.begin(), v.end(), [](double &c){ c /= 2.0; });
    return v;
}



}  // namespace measurement
}  // namespace nuis
