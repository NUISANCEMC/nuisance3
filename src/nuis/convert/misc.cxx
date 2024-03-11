namespace plotly {
std::string to_1D_json(HistFrame const &hf) {
  boost::json::array data;

  for (HistFrame::column_t col = 0; col < hf.column_info.size(); ++col) {
    boost::json::object data_instance;
    data_instance["type"] = "scatter";

    boost::json::array x;
    auto bcs = get_bin_centers1D(hf.binning.bins);
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
} // namespace plotly

namespace matplotlib {

// motivated from here:
// https://matplotlib.org/stable/api/_as_gen/matplotlib.axes.Axes.pcolormesh.html
std::map<std::string, Eigen::ArrayXXd>
to_pcolormesh_data(HistFrame const &hf, HistFrame::column_t colid) {
  auto nbins = hf.binning.bins.size();
  Eigen::ArrayXXd X = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd Y = Eigen::ArrayXXd::Zero(2 * nbins, 2);
  Eigen::ArrayXXd C = Eigen::ArrayXXd::Zero((2 * nbins) - 1, 1);
  for (size_t bi = 0; bi < nbins; ++bi) {
    X(2 * bi, 0) = hf.binning.bins[bi][0].min;
    Y(2 * bi, 0) = hf.binning.bins[bi][1].min;

    X(2 * bi + 1, 0) = hf.binning.bins[bi][0].min;
    Y(2 * bi + 1, 0) = hf.binning.bins[bi][1].max;

    X(2 * bi, 1) = hf.binning.bins[bi][0].max;
    Y(2 * bi, 1) = hf.binning.bins[bi][1].min;

    X(2 * bi + 1, 1) = hf.binning.bins[bi][0].max;
    Y(2 * bi + 1, 1) = hf.binning.bins[bi][1].max;

    C(2 * bi, 0) = hf.contents(bi, colid);
    if ((2 * bi + 2) != (2 * nbins)) {
      C(2 * bi + 1, 0) = hf.contents(bi, colid);
    }
  }
  return {{"X", X}, {"Y", Y}, {"C", C}};
}
} // namespace matplotlib
