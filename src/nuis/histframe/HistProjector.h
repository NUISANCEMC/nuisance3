#pragma once

#include "nuis/histframe/Binning.h"
#include "nuis/histframe/HistFrame.h"

#include "fmt/ranges.h"

namespace nuis {
HistFrame Project(HistFrame const &hf, std::vector<size_t> proj_to_axes);
} // namespace nuis
