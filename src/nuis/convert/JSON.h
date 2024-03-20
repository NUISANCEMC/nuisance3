#pragma once

#include "nuis/histframe/BinnedValues.h"

#include "boost/json.hpp"

namespace nuis {
// boost::json overloads
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                Binning const &bi);

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                BinnedValues const &hf);
} // namespace nuis