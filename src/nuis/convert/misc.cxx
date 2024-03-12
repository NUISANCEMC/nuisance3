#include "nuis/convert/misc.h"

#include "nuis/binning/utility.h"

#include "boost/json.hpp"

namespace nuis {
template <typename T> std::string to_plotly1D(T const &hf) {
  boost::json::array data;

  for (typename T::column_t col = 0; col < hf.column_info.size(); ++col) {
    boost::json::object data_instance;
    data_instance["type"] = "scatter";

    boost::json::array x;
    auto bcs = get_bin_centers1D(hf.binning->bins);
    std::copy(bcs.begin(), bcs.end(), std::back_inserter(x));
    data_instance["x"] = x;

    boost::json::array col_contents;

    if constexpr (std::is_same_v<T, HistFrame>) {
      std::copy(hf.sumweights.col(col).begin(), hf.sumweights.col(col).end(),
                std::back_inserter(col_contents));
    } else {
      std::copy(hf.values.col(col).begin(), hf.values.col(col).end(),
                std::back_inserter(col_contents));
    }
    data_instance["y"] = col_contents;
    data.push_back(data_instance);
  }

  return boost::json::serialize(data);
}

// motivated from here:
// https://matplotlib.org/stable/api/_as_gen/matplotlib.axes.Axes.pcolormesh.html
template <typename T>
std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(T const &hf, typename T::column_t colid) {
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

    if constexpr (std::is_same_v<T, HistFrame>) {
      C(2 * bi, 0) = hf.sumweights(bi, colid);
      if ((2 * bi + 2) != (2 * nbins)) {
        C(2 * bi + 1, 0) = hf.sumweights(bi, colid);
      }
    } else {
      C(2 * bi, 0) = hf.values(bi, colid);
      if ((2 * bi + 2) != (2 * nbins)) {
        C(2 * bi + 1, 0) = hf.values(bi, colid);
      }
    }
  }
  return {{"X", X}, {"Y", Y}, {"C", C}};
}

std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(HistFrame const &hf, HistFrame::column_t colid) {
  return to_mpl_pcolormesh(hf, colid);
}
std::map<std::string, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValues const &hf, BinnedValues::column_t colid) {
  return to_mpl_pcolormesh(hf, colid);
}

} // namespace nuis
