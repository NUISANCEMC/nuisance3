#include "nuis/convert/json.h"

#include "boost/json/src.hpp"

namespace nuis {

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                Binning const &bi) {
  boost::json::array bins_arr;
  bins_arr.resize(bi.bins.size());
  for (size_t i = 0; i < bi.bins.size(); ++i) {
    boost::json::array ax_exts;
    ax_exts.resize(bi.bins[i].size());
    for (size_t j = 0; j < bi.bins[i].size(); ++j) {
      ax_exts[j] = {{"low", bi.bins[i][j].low}, {"high", bi.bins[i][j].high}};
    }
    bins_arr[i] = ax_exts;
  }

  jv = {{"independent_axis_labels", boost::json::value_from(bi.axis_labels)},
        {"bins", bins_arr}};
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf) {
  boost::json::object hist_frame;
  hist_frame["binning"] = boost::json::value_from(*hf.binning);

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