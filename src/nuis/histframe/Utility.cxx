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
    hf.Fill(evproj, weights[i], col);
  }
}

Eigen::ArrayXd Data(HistFrame const &hf) { return hf.content.col(0); }
Eigen::ArrayXd DataError(HistFrame const &hf) {
  return hf.content.col(0).sqrt();
}

Eigen::ArrayXd MC(HistFrame const &hf) { return hf.content.col(1); }
Eigen::ArrayXd MCStatisticalError(HistFrame const &hf) {
  return hf.content.col(1).sqrt();
}

Eigen::ArrayXd Column(HistFrame const &hf, std::string const &name) {
  return hf.content.col(hf.GetColumnIndex(name));
}
Eigen::ArrayXd ColumnStatisticalError(HistFrame const &hf,
                                      std::string const &name) {
  return hf.content.col(hf.GetColumnIndex(name)).sqrt();
}

// Scales all columns except the data column
void ScaleAllMC(HistFrame &hf, double s, bool divide_by_cell_area) {
  for (int ci = 1; ci < hf.content.cols(); ++ci) {
    for (int ri = 0; ri < hf.content.rows(); ++ri) {
      double area = 1;
      if (divide_by_cell_area) {
        for (auto const &binrange : hf.binning.bin_info.extents[ri]) {
          area *= binrange.width();
        }
      }

      hf.content(ri, ci) *= s / area;
      hf.variance(ri, ci) *= std::pow(s / area, 2);
    }
  }
}

namespace Bins {
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                BinningInfo const &bi) {
  boost::json::array bins_arr;
  bins_arr.resize(bi.extents.size());
  for (size_t i = 0; i < bi.extents.size(); ++i) {
    boost::json::array ax_exts;
    ax_exts.resize(bi.extents[i].size());
    for (size_t j = 0; j < bi.extents[i].size(); ++j) {
      ax_exts[j] = {{"min", bi.extents[i][j].min},
                    {"max", bi.extents[i][j].max}};
    }
    bins_arr[i] = ax_exts;
  }

  jv = {{"independent_axis_labels", boost::json::value_from(bi.axis_labels)},
        {"bins", bins_arr}};
}
} // namespace Bins

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf) {
  boost::json::object hist_frame;
  hist_frame["binning"] = boost::json::value_from(hf.binning.bin_info);

  for (int i = 0; i < hf.content.cols(); ++i) {
    std::string key_name =
        hf.column_info.size() > size_t(i)
            ? hf.column_info[i].name.size()
                  ? hf.column_info[i].name
                  : (std::string("column_") + std::to_string(i))
            : (std::string("column_") + std::to_string(i));

    boost::json::object column;
    column["independent_axis_label"] =
        hf.column_info.size() > size_t(i)
            ? hf.column_info[i].independent_axis_label
            : "";

    boost::json::array content, variance;
    content.resize(hf.content.rows());
    variance.resize(hf.content.rows());
    for (int j = 0; j < hf.content.rows(); ++j) {
      content[j] = hf.content(j, i);
      variance[j] = hf.variance(j, i);
    }
    column["content"] = content;
    column["variance"] = variance;
    hist_frame[key_name] = column;
  }
  jv = hist_frame;
}

} // namespace nuis