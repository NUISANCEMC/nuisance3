#pragma once

#include "nuis/histframe/BinnedValues.h"
#include "nuis/histframe/HistFrame.h"

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> const &proj_to_axes);
HistFrame Project(HistFrame const &hf, size_t proj_to_axis);
BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes);
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis);

} // namespace nuis