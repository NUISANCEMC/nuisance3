#pragma once

#include "nuis/histframe/Binning.h"
#include "nuis/histframe/HistFrame.h"

#include "fmt/ranges.h"

namespace nuis {
namespace Bins {
std::vector<Bins::BinningInfo::extent>
GetAxisExtents(BinningInfo const &bin_info, size_t proj_to_axis);
} // namespace Bins
HistFrame Project1D(HistFrame const &hf, size_t proj_to_axis);
} // namespace nuis
