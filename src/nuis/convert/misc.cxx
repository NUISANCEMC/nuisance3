#include "nuis/convert/misc.h"

#include "nuis/binning/utility.h"

#include "boost/json.hpp"

#include "fmt/core.h"
#include "fmt/ranges.h"

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

std::tuple<Eigen::ArrayXXd, Eigen::ArrayXXd, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValuesBase const &bvb, BinnedValuesBase::column_t col) {

  auto const &[x_edges, y_edges] = get_rectilinear_grid(bvb.binning->bins);

  Eigen::ArrayXXd X, Y, C;
  X = Eigen::ArrayXXd::Zero(x_edges.size(), y_edges.size());
  Y = Eigen::ArrayXXd::Zero(x_edges.size(), y_edges.size());
  C = Eigen::ArrayXXd::Zero(x_edges.size() - 1, y_edges.size() - 1);

  auto bin_contents = bvb.get_bin_contents();

  for (size_t x = 0; x < x_edges.size(); ++x) {
    for (size_t y = 0; y < y_edges.size(); ++y) {
      X(x, y) = x_edges[x];
      Y(x, y) = y_edges[y];
      if (((x + 1) < x_edges.size()) && ((y + 1) < y_edges.size())) {

        auto bi_it =
            bvb.binning->find_bin({(x_edges[x] + x_edges[x + 1]) / 2.0,
                                   (y_edges[y] + y_edges[y + 1]) / 2.0});

        if (bi_it == Binning::npos) {
          C(x, y) = std::numeric_limits<double>::quiet_NaN();

        } else {
          C(x, y) = bin_contents(bi_it, col);
        }
      }
    }
  }

  return {X, Y, C};
}

} // namespace nuis
