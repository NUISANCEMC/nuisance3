#pragma once

#include "nuis/histframe/HistFrame.h"

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> const& proj_to_axes);
HistFrame Project(HistFrame const &hf, size_t proj_to_axis);

} // namespace nuis