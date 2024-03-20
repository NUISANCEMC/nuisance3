#pragma once

#include "nuis/histframe/BinnedValues.h"

#include "Eigen/Dense"

#include <map>
#include <string>

namespace nuis {
std::string to_plotly1D(BinnedValuesBase const &);

std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValuesBase const &,
                  BinnedValuesBase::column_t colid = 0);

} // namespace nuis