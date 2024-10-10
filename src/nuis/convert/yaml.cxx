#include "nuis/convert/yaml.h"

#include "spdlog/fmt/bundled/core.h"

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
template <> struct convert<nuis::SingleExtent> {
  static Node encode(nuis::SingleExtent const &rhs) {
    YAML::Node out;
    out["low"] = fmt::format("{:.4}", rhs.low);
    out["high"] = fmt::format("{:.4}", rhs.high);
    return out;
  }

  static bool decode(const Node &node, nuis::SingleExtent &rhs) {
    if (!node.IsMap()) {
      return false;
    }

    rhs.low = node["low"].as<double>();
    rhs.high = node["high"].as<double>();
    return true;
  }
};

template <> struct convert<nuis::BinningPtr> {
  static Node encode(nuis::BinningPtr const &rhs) {
    YAML::Node out;
    for (size_t ax_it = 0; ax_it < rhs->axis_labels.size(); ++ax_it) {
      YAML::Node var;
      var["header"] = std::map<std::string, std::string>{
          {"name", rhs->axis_labels[ax_it]}, {"units", ""}};
      for (auto const &bin : rhs->bins) {
        var["values"].push_back(bin[ax_it]);
      }
      out.push_back(var);
    }
    return out;
  }

  static bool decode(const Node &node, nuis::BinningPtr &rhs) {
    if (!node.IsSequence() || !node.size()) {
      return false;
    }
    std::vector<nuis::Binning::BinExtents> bins;
    std::vector<std::string> axis_labels;
    for (size_t ni = 0; ni < node.size(); ++ni) { // each independent_variable
      auto const &indep_var = node[ni];

      if (!indep_var["values"] || !indep_var["values"].IsSequence() ||
          !indep_var["values"].size()) {
        return false;
      }

      if (!bins.size()) { // build the bin shape in nuis::Binning ordering
        bins.resize(indep_var["values"].size());
        for (size_t bi = 0; bi < bins.size(); ++bi) {
          bins[bi].resize(node.size());
        }
      }

      if (indep_var["header"] && indep_var["header"]["name"]) {
        axis_labels.push_back(indep_var["header"]["name"].as<std::string>());
      } else {
        axis_labels.emplace_back();
      }

      for (size_t bi = 0; bi < indep_var["values"].size(); ++bi) {
        auto const &bin = indep_var["values"][bi];
        bins[bi][ni] = bin.as<nuis::SingleExtent>();
      }
    }

    rhs = nuis::Binning::from_extents(bins, axis_labels);
    return true;
  }
};

template <> struct convert<nuis::BinnedValues> {
  static Node encode(nuis::BinnedValues const &rhs) {

    YAML::Node out;
    out["independent_variables"] = rhs.binning;

    YAML::Node header;
    header["name"] = rhs.column_info[0].column_label;

    YAML::Node dvar;
    dvar["header"] = header;

    for (size_t i = 0; i < rhs.binning->bins.size(); ++i) {
      YAML::Node val;
      val["value"] = rhs.values(i, 0);

      YAML::Node staterr;
      staterr["symerror"] = rhs.errors(i, 0);
      staterr["label"] = "stat";
      val["errors"].push_back(staterr);
      dvar["values"].push_back(val);
    }

    out["dependent_variables"].push_back(dvar);
    return out;
  }

  static bool decode(const Node &node, nuis::BinnedValues &rhs) {
    if (!node.IsMap() || !node["independent_variables"] ||
        !node["dependent_variables"]) {
      return false;
    }

    nuis::BinnedValues hf;

    hf.binning = node["independent_variables"].as<nuis::BinningPtr>();

    for (size_t dv_it = 0; dv_it < node["dependent_variables"].size();
         ++dv_it) {
      auto const &dvar = node["dependent_variables"][dv_it];
      hf.add_column(dvar["header"]["name"].as<std::string>());
    }

    hf.values =
        Eigen::ArrayXXd::Zero(hf.binning->bins.size(), hf.column_info.size());
    hf.errors =
        Eigen::ArrayXXd::Zero(hf.binning->bins.size(), hf.column_info.size());

    for (size_t dv_it = 0; dv_it < node["dependent_variables"].size();
         ++dv_it) {
      auto const &dvar = node["dependent_variables"][dv_it];
      for (size_t bi = 0; bi < dvar["values"].size(); ++bi) {
        auto const &val = dvar["values"][bi];
        hf.values(bi, dv_it) = val["value"].as<double>();

        if (val["errors"] && val["errors"].size()) {
          hf.errors(bi, dv_it) = val["errors"][0]["symerror"].as<double>();
        }
      }
    }

    rhs = hf;

    return true;
  }
};
} // namespace YAML

namespace nuis {
YAML::Node to_yaml(BinnedValues const &hf) {
  return YAML::convert<nuis::BinnedValues>::encode(hf);
}
std::string to_yaml_str(BinnedValues const &hf) {
  return str_via_ss(to_yaml(hf));
}

BinnedValues from_yaml(YAML::Node const &yhf) { return yhf.as<BinnedValues>(); }
BinnedValues from_yaml_str(std::string const &shf) {
  return YAML::Load(shf).as<BinnedValues>();
}

DECLARE_NUISANCE_EXCEPT(InvalidYAML);

Eigen::ArrayXXd covar_from_yaml(YAML::Node const &node) {
  if (!node.IsMap() || !node["independent_variables"] ||
      !node["dependent_variables"]) {
    throw InvalidYAML() << "Could not parse covariance from input YAML";
  }

  auto bin_i = node["independent_variables"][0]["values"];
  auto bin_j = node["independent_variables"][1]["values"];

  size_t max_i = 0, max_j = 0;

  auto covar_vals = node["dependent_variables"][0]["values"];

  for (size_t ci = 0; ci < covar_vals.size(); ++ci) {
    max_i = std::max(max_i, bin_i[ci]["value"].as<size_t>());
    max_j = std::max(max_j, bin_j[ci]["value"].as<size_t>());
  }

  if (max_i != max_j) {
    throw InvalidYAML() << "When parsing covariance from input YAML, "
                           "independent_variable[0] had "
                        << max_i
                        << " values, and independent_variable[1] had "
                        << max_j << " values";
  }

  Eigen::ArrayXXd covar = Eigen::ArrayXXd::Zero(max_i+1, max_j+1);

  for (size_t ci = 0; ci < covar_vals.size(); ++ci) {
    size_t i = bin_i[ci]["value"].as<size_t>();
    size_t j = bin_j[ci]["value"].as<size_t>();
    covar(i, j) = covar_vals[ci]["value"].as<double>();
  }

  return covar;
}

Eigen::ArrayXXd covar_from_yaml_str(std::string const &shf) {
  return covar_from_yaml(YAML::Load(shf));
}

} // namespace nuis
