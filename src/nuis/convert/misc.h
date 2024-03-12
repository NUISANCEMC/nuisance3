#pragma once

#include "nuis/histframe/HistFrame.h"

#include "Eigen/Dense"

#include <map>
#include <string>

namespace nuis {
std::string to_plotly1D(HistFrame const &hf);
std::string to_plotly1D(BinnedValues const &hf);

std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(HistFrame const &hf, HistFrame::column_t colid = 0);
std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValues const &hf, BinnedValues::column_t colid = 0);

} // namespace nuis