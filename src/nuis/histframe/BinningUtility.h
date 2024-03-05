#pragma once

#include "nuis/histframe/Binning.h"

#include <iostream>

namespace nuis {
std::vector<Binning::BinExtents>
stable_sort(std::vector<Binning::BinExtents> bins);
std::vector<Binning::BinExtents> unique(std::vector<Binning::BinExtents> bins);
bool bins_overlap(Binning::BinExtents const &a, Binning::BinExtents const &b);
std::vector<Binning::BinExtents>
project_to_unique_bins(std::vector<Binning::BinExtents> const &bins,
                       std::vector<size_t> const &proj_to_axes);
bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          std::vector<size_t> const &proj_to_axes = {});
bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          size_t proj_to_axis);
} // namespace nuis