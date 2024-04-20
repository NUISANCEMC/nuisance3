#include "nuis/convert/misc.h"

#include "nuis/binning/utility.h"

#include "boost/json.hpp"

namespace nuis {

std::string to_plotly1D(BinnedValuesBase const &bvb) {
  boost::json::array data;

  auto values = bvb.get_bin_contents();

  for (BinnedValuesBase::column_t col = 0; col < bvb.column_info.size();
       ++col) {
    boost::json::object data_instance;
    data_instance["type"] = "scatter";

    boost::json::array x;
    auto bcs = get_bin_centers1D(bvb.binning->bins);
    std::copy(bcs.begin(), bcs.end(), std::back_inserter(x));
    data_instance["x"] = x;

    boost::json::array col_contents;

    std::copy(values.col(col).begin(), values.col(col).end(),
              std::back_inserter(col_contents));

    data_instance["y"] = col_contents;
    data.push_back(data_instance);
  }

  return boost::json::serialize(data);
}

} // namespace nuis
