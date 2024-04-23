#pragma once

#include "nuis/histframe/HistFrame.h"

#include "nuis/eventframe/EventFrameGen.h"

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> const &proj_to_axes,
                  bool result_has_binning = true);
HistFrame Project(HistFrame const &hf, size_t proj_to_axis,
                  bool result_has_binning = true);
BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes,
                     bool result_has_binning = true);
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis,
                     bool result_has_binning = true);

HistFrame Slice(HistFrame const &hf, size_t ax,
                std::array<double, 2> slice_range,
                bool exclude_range_end_bin = false,
                bool result_has_binning = true);
HistFrame Slice(HistFrame const &hf, size_t ax, double slice_val,
                bool result_has_binning = true);
BinnedValues Slice(BinnedValues const &hf, size_t ax,
                   std::array<double, 2> slice_range,
                   bool exclude_range_end_bin = false,
                   bool result_has_binning = true);
BinnedValues Slice(BinnedValues const &hf, size_t ax, double slice_val,
                   bool result_has_binning = true);

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesBase const &);

} // namespace nuis