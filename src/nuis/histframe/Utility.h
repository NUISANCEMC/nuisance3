#pragma once

#include "nuis/histframe/HistFrame.h"

#include "boost/json.hpp"

namespace nuis {

void FillHistFromEventColumns(HistFrame &hf, Eigen::ArrayXd const &weights,
                              Eigen::ArrayXXd const &projections,
                              HistFrame::column_t col = 1);

Eigen::ArrayXd Data(HistFrame const &hf);
Eigen::ArrayXd DataError(HistFrame const &hf);

Eigen::ArrayXd MC(HistFrame const &hf);
Eigen::ArrayXd MCStatisticalError(HistFrame const &hf);
Eigen::ArrayXd Column(HistFrame const &hf, std::string const &name);
Eigen::ArrayXd ColumnStatisticalError(HistFrame const &hf,
                                      std::string const &name);

void ScaleAllMC(HistFrame &hf, double s, bool divide_by_cell_area = false);

namespace Bins {
//boost::json overloads
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                BinningInfo const &bi);
} // namespace Bins

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf);

} // namespace nuis
