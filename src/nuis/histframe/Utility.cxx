#include "nuis/histframe/Utility.h"

#include "nuis/histframe/BinningUtility.h"

#include "nuis/log.txx"

#include "boost/json/src.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

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

// independent_variables:
// - header: {name: Leading dilepton PT, units: GEV}
//   values:
//   - {low: 0, high: 60}
//   - {low: 60, high: 100}
//   - {low: 100, high: 200}
//   - {low: 200, high: 600}
// dependent_variables:
// - header: {name: 10**6 * 1/SIG(fiducial) * D(SIG(fiducial))/DPT, units:
// GEV**-1}
//   qualifiers:
//   - {name: RE, value: P P --> Z0 < LEPTON+ LEPTON- > Z0 < LEPTON+ LEPTON- >
//   X}
//   - {name: SQRT(S), units: GEV, value: 7000}
//   values:
//   - value: 7000
//     errors:
//     - {symerror: 1100, label: stat}
//     - {symerror: 79, label: 'sys,detector'}
//     - {symerror: 15, label: 'sys,background'}

namespace YAML {
template <> struct convert<nuis::Binning::SingleExtent> {
  static Node encode(nuis::Binning::SingleExtent const &rhs) {
    YAML::Node out;
    out["low"] = rhs.min;
    out["high"] = rhs.max;
    return out;
  }
};
template <> struct convert<nuis::Binning> {
  static Node encode(nuis::Binning const &rhs) {
    YAML::Node out;
    for (size_t ax_it = 0; ax_it < rhs.axis_labels.size(); ++ax_it) {
      YAML::Node var;
      var["header"] = std::map<std::string, std::string>{
          {"name", rhs.axis_labels[ax_it]}, {"units", ""}};
      for (auto const &bin : rhs.bins) {
        var["values"].push_back(bin[ax_it]);
      }
      out.push_back(var);
    }
    return out;
  }
};
template <> struct convert<nuis::HistFrame> {
  static Node encode(nuis::HistFrame const &rhs) {

    YAML::Node out;
    out["independent_variables"] = rhs.binning;

    YAML::Node header;
    header["name"] = rhs.column_info[0].dependent_axis_label;

    YAML::Node ivar;
    ivar["header"] = header;

    for (size_t i = 0; i < rhs.binning.bins.size(); ++i) {
      YAML::Node val;
      val["value"] = rhs.contents(i, 0);

      YAML::Node staterr;
      staterr["symerror"] = std::sqrt(rhs.variance(i, 0));
      staterr["label"] = "stat";
      val["errors"].push_back(staterr);
      ivar["values"].push_back(val);
    }

    out["dependent_variables"].push_back(ivar);
    return out;
  }
};
} // namespace YAML

namespace nuis {
YAML::Node to_yaml(HistFrame const &hf) {
  return YAML::convert<nuis::HistFrame>::encode(hf);
}
std::string to_yaml_str(HistFrame const &hf) { return str_via_ss(to_yaml(hf)); }

} // namespace nuis
