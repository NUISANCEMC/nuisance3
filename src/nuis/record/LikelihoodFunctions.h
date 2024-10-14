#pragma once

#include "nuis/record/Comparison.h"

#include "spdlog/fmt/bundled/core.h"

#include "Eigen/Dense"

#include <functional>

namespace nuis {
namespace likelihood {
using func = std::function<double(Comparison const &)>;

inline func chi2_inv_covariance(Eigen::MatrixXd const &inv_covariance) {
  return [=](Comparison const &comp) -> double {
    size_t nbins = 0;
    for (auto const &d : comp.data) {
      nbins += d.values.rows();
    }

    Eigen::VectorXd diff_vect = Eigen::VectorXd::Zero(nbins);

    // assumes first column, could extend to one that returns a vector of chi2
    // and operates colwise
    size_t first_bin = 0;
    for (size_t i = 0; i < comp.data.size(); ++i) {
      diff_vect.segment(first_bin, comp.data[i].values.rows()) =
          comp.data[i].values.col(0) - comp.predictions[i].values.col(0);

      first_bin += comp.data[i].values.rows();
    }

    return diff_vect.transpose() * inv_covariance * diff_vect;
  };
}
} // namespace likelihood
} // namespace nuis