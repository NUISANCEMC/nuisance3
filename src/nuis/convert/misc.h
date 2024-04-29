#pragma once

#include "nuis/histframe/BinnedValues.h"

#include "Eigen/Dense"

#include <map>
#include <string>

namespace nuis {
std::string to_plotly1D(BinnedValuesBase const &);
std::tuple<Eigen::ArrayXXd, Eigen::ArrayXXd, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValuesBase const &, BinnedValuesBase::column_t col = 0);
} // namespace nuis