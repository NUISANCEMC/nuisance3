#pragma once

#include "nuis/histframe/HistFrame.h"

#include "boost/json.hpp"

namespace nuis {

std::string plotly1D(HistFrame const &hf);
std::string plotly2D(HistFrame const &hf);

// boost::json overloads
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                Binning const &bi);

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf);

} // namespace nuis
