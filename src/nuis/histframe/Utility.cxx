#include "nuis/histframe/Utility.h"

#include "spdlog/spdlog.h"

#include "boost/json/src.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

void FillHistFromEventColumns(HistFrame &hf, Eigen::ArrayXd const &weights,
                              Eigen::ArrayXXd const &projections,
                              HistFrame::column_t col) {
  int nrows = weights.rows();
  int ncols = projections.cols();

  std::vector<double> evproj(ncols, 0);

  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < ncols; ++j) {
      evproj[j] = projections(i, j);
    }
    hf.fill(evproj, weights[i], col);
  }
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                Binning const &bi) {
  boost::json::array bins_arr;
  bins_arr.resize(bi.bins.size());
  for (size_t i = 0; i < bi.bins.size(); ++i) {
    boost::json::array ax_exts;
    ax_exts.resize(bi.bins[i].size());
    for (size_t j = 0; j < bi.bins[i].size(); ++j) {
      ax_exts[j] = {{"min", bi.bins[i][j].min}, {"max", bi.bins[i][j].max}};
    }
    bins_arr[i] = ax_exts;
  }

  jv = {{"independent_axis_labels", boost::json::value_from(bi.axis_labels)},
        {"bins", bins_arr}};
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf) {
  boost::json::object hist_frame;
  hist_frame["binning"] = boost::json::value_from(hf.binning);

  for (int i = 0; i < hf.contents.cols(); ++i) {
    std::string key_name =
        hf.column_info.size() > size_t(i)
            ? hf.column_info[i].name.size()
                  ? hf.column_info[i].name
                  : (std::string("column_") + std::to_string(i))
            : (std::string("column_") + std::to_string(i));

    boost::json::object column;
    column["dependent_axis_label"] =
        hf.column_info.size() > size_t(i)
            ? hf.column_info[i].dependent_axis_label
            : "";

    boost::json::array contents, variance;
    contents.resize(hf.contents.rows());
    variance.resize(hf.contents.rows());
    for (int j = 0; j < hf.contents.rows(); ++j) {
      contents[j] = hf.contents(j, i);
      variance[j] = hf.variance(j, i);
    }
    column["contents"] = contents;
    column["variance"] = variance;
    hist_frame[key_name] = column;
  }
  jv = hist_frame;
}

} // namespace nuis