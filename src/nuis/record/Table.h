#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "nuis/record/Comparison.h"

namespace nuis {

using ClearFunc =
    std::function<void(Comparison&)>;

using ProjectFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using FullProjectFunc =
    std::function<std::vector<double>(HepMC3::GenEvent const &)>;

using WeightFunc =
    std::function<double(HepMC3::GenEvent const &)>;

using SelectFunc =
    std::function<int(HepMC3::GenEvent const &)>;

using FinalizeFunc =
    std::function<void(Comparison&, const double)>;

using LikelihoodFunc =
    std::function<double(Comparison const&)>;

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
    std::shared_ptr<Comparison> blueprint;
    Comparison comparison() {
        return *blueprint;
    }

    ClearFunc clear;
    SelectFunc select;
    WeightFunc weight;
    std::vector<ProjectFunc> projections;
    FullProjectFunc project;
    FinalizeFunc finalize;
    LikelihoodFunc likeihood;

    // I also want the ability to do table.add_column for direct blueprint updates.
    template<typename... TS>
    auto add_column(TS&&... args)
        {return blueprint->add_column(std::forward<TS>(args)...);};

    template<typename... TS>
    auto find_column_index(TS&&... args)
        {return blueprint->
            find_column_index(std::forward<TS>(args)...);};

};
}
