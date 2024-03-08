#include "nuis/histframe/Utility.h"

#include "nuis/histframe/BinningUtility.h"

#include "spdlog/spdlog.h"

#include "boost/json/src.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

// {
//       "type": "scatter",
//       "x": [
//         "2007-12-01",
//         "2008-12-01",
//         "2009-12-01",
//         "2010-12-01",
//         "2011-12-01",
//         "2012-12-01",
//         "2013-12-01",
//         "2014-12-01",
//         "2015-12-01"
//       ],
//       "y": [
//         "0",
//         "45560506.663365364",
//         "91145081.21192169",
//         "232447635.15836716",
//         "580348915.5698586",
//         "1182888421.2842617",
//         "1928559640.2194986",
//         "2578825762.2643065",
//         "3022276546.8773637"
//       ]
//     }

std::string plotly1D(HistFrame const &hf) {
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

// var data = [
//   {
//     z: [[1, null, 30, 50, 1], [20, 1, 60, 80, 30], [30, 60, 1, -10, 20]],
//     x: ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'],
//     y: ['Morning', 'Afternoon', 'Evening'],
//     type: 'heatmap',
//     hoverongaps: false
//   }
// ];

std::string plotly2D(HistFrame const &hf) {
  boost::json::array data;

  for (HistFrame::column_t col = 0; col < hf.column_info.size(); ++col) {
    boost::json::object data_instance;
    data_instance["type"] = "heatmap";
    data_instance["hoverongaps"] = false;

    boost::json::array x, y;
    auto bcs = get_bin_centers(hf.binning.bins);
    for (auto const &bc : bcs) {
      x.push_back(bc[0]);
      y.push_back(bc[1]);
    }
    data_instance["x"] = x;
    data_instance["y"] = y;

    boost::json::array col_contents;
    std::copy(hf.contents.col(col).begin(), hf.contents.col(col).end(),
              std::back_inserter(col_contents));
    data_instance["z"] = col_contents;
    data.push_back(data_instance);
  }

  return boost::json::serialize(data);
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