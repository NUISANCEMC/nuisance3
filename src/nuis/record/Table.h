#pragma once

#include <vector>
#include <memory>

#include "nuis/record/ComparisonFrame.h"

namespace nuis {

using ClearFunc =
    std::function<void(ComparisonFrame&)>;

using ProjectFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using FullProjectFunc =
    std::function<std::vector<double>(HepMC3::GenEvent const &)>;

using WeightFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using SelectFunc =
    std::function<int(HepMC3::GenEvent const &)>;

using FinalizeFunc =
    std::function<void(ComparisonFrame&, const double)>;

using LikelihoodFunc =
    std::function<double(ComparisonFrame const&)>;

// The point in a table is to map
// the hist frame associated with a given RecordFactory output
// with the functions needed to project
struct Table {

    // Here we purposefully keep a full template
    // so that people can change the binning/data
    // on the fly and all others get propagated.

    // This also allows Table.blueprint->add_column()
    // And all spawned objects get the same.
    // Don't love the . -> requirement
    std::shared_ptr<ComparisonFrame> blueprint;
    ComparisonFrame comparison() {
        return *blueprint;
    }

    ClearFunc clear;
    SelectFunc select;
    WeightFunc weight;
    std::vector<ProjectFunc> projections;
    FullProjectFunc project;
    FinalizeFunc finalize;
    LikelihoodFunc likeihood;
};
}
