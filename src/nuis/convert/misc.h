#pragma once

#include "nuis/histframe/BinnedValues.h"

#include "Eigen/Dense"

#include <map>
#include <string>

namespace nuis {
std::string to_plotly1D(BinnedValuesBase const &);
} // namespace nuis