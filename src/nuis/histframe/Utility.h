#pragma once

#include "nuis/histframe/HistFrame.h"

#include "boost/json.hpp"

namespace nuis {

void FillHistFromEventColumns(HistFrame &hf, Eigen::ArrayXd const &weights,
                              Eigen::ArrayXXd const &projections,
                              HistFrame::column_t col = 1);

namespace Bins {
//boost::json overloads
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                BinningInfo const &bi);
} // namespace Bins

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf);

} // namespace nuis
