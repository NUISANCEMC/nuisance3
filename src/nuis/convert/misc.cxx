#include "nuis/convert/misc.h"

#include "nuis/binning/utility.h"

#include "boost/json.hpp"

namespace nuis {
std::string to_plotly1D(HistFrame const &hf) {
  boost::json::array data;

  for (HistFrame::column_t col = 0; col < hf.column_info.size(); ++col) {
    boost::json::object data_instance;
    data_instance["type"] = "scatter";

    boost::json::array x;
    auto bcs = get_bin_centers1D(hf.binning->bins);
    std::copy(bcs.begin(), bcs.end(), std::back_inserter(x));
    data_instance["x"] = x;

    boost::json::array col_contents;
    std::copy(hf.contents.col(col).begin(), hf.contents.col(col).end(),
              std::back_inserter(col_contents));
    data_instance["y"] = col_contents;
    data.push_back(data_instance);
  }

  return boost::json::serialize(data);
}

// motivated from here:
// https://matplotlib.org/stable/api/_as_gen/matplotlib.axes.Axes.pcolormesh.html
std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(HistFrame const &hf, HistFrame::column_t colid) {
  auto nbins = hf.binning->bins.size();
  Eigen::ArrayXXd X = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd Y = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd C = Eigen::ArrayXXd::Zero((2 * nbins) - 1, 1);
  for (size_t bi = 0; bi < nbins; ++bi) {
    X(2 * bi, 0) = hf.binning->bins[bi][0].low;
    Y(2 * bi, 0) = hf.binning->bins[bi][1].low;

    X(2 * bi + 1, 0) = hf.binning->bins[bi][0].low;
    Y(2 * bi + 1, 0) = hf.binning->bins[bi][1].high;

    X(2 * bi, 1) = hf.binning->bins[bi][0].high;
    Y(2 * bi, 1) = hf.binning->bins[bi][1].low;

    X(2 * bi + 1, 1) = hf.binning->bins[bi][0].high;
    Y(2 * bi + 1, 1) = hf.binning->bins[bi][1].high;

    C(2 * bi, 0) = hf.contents(bi, colid);
    if ((2 * bi + 2) != (2 * nbins)) {
      C(2 * bi + 1, 0) = hf.contents(bi, colid);
    }
  }
  return {{"X", X}, {"Y", Y}, {"C", C}};
}
} // namespace nuis
