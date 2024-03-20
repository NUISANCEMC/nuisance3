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

// motivated from here:
// https://matplotlib.org/stable/api/_as_gen/matplotlib.axes.Axes.pcolormesh.html
std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValuesBase const &bvb,
                  BinnedValuesBase::column_t colid) {
  auto nbins = bvb.binning->bins.size();
  Eigen::ArrayXXd X = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd Y = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd C = Eigen::ArrayXXd::Zero((2 * nbins) - 1, 1);

  auto values = bvb.get_bin_contents();

  for (size_t bi = 0; bi < nbins; ++bi) {
    X(2 * bi, 0) = bvb.binning->bins[bi][0].low;
    Y(2 * bi, 0) = bvb.binning->bins[bi][1].low;

    X(2 * bi + 1, 0) = bvb.binning->bins[bi][0].low;
    Y(2 * bi + 1, 0) = bvb.binning->bins[bi][1].high;

    X(2 * bi, 1) = bvb.binning->bins[bi][0].high;
    Y(2 * bi, 1) = bvb.binning->bins[bi][1].low;

    X(2 * bi + 1, 1) = bvb.binning->bins[bi][0].high;
    Y(2 * bi + 1, 1) = bvb.binning->bins[bi][1].high;

    C(2 * bi, 0) = values(bi, colid);
    if ((2 * bi + 2) != (2 * nbins)) {
      C(2 * bi + 1, 0) = values(bi, colid);
    }
  }
  return {{"X", X}, {"Y", Y}, {"C", C}};
}

} // namespace nuis
