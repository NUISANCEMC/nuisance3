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

  jv = {{"incolumn_labels", boost::json::value_from(bi.axis_labels)},
        {"bins", bins_arr}};
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                BinnedValues const &bv) {
  boost::json::object hist_frame;
  hist_frame["binning"] = boost::json::value_from(*bv.binning);

  for (int i = 0; i < bv.values.cols(); ++i) {
    std::string key_name =
        bv.column_info.size() > size_t(i)
            ? bv.column_info[i].name.size()
                  ? bv.column_info[i].name
                  : (std::string("column_") + std::to_string(i))
            : (std::string("column_") + std::to_string(i));

    boost::json::object column;
    column["column_label"] =
        bv.column_info.size() > size_t(i)
            ? bv.column_info[i].column_label
            : "";

    boost::json::array values, errors;
    values.resize(bv.values.rows());
    errors.resize(bv.values.rows());
    for (int j = 0; j < bv.values.rows(); ++j) {
      values[j] = bv.values(j, i);
      errors[j] = bv.errors(j, i);
    }
    column["values"] = values;
    column["errors"] = errors;
    hist_frame[key_name] = column;
  }
  jv = hist_frame;
}
} // namespace nuis