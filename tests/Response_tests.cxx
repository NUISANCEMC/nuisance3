#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "nuis/response/FramedResponse.h"

#include "spdlog/spdlog.h"

#include <cassert>
#include <random>
#include <iostream>

TEST_CASE("FramedResponse", "[Response]") {

  Eigen::ArrayXd x{{0, 1, 2, 5.0 / 2.0}};
  Eigen::ArrayXXd y{{0, 1, 8, 9}};
  nuis::NaturalCubicFrameSplineXd sp(x);
  sp.build(y);
  std::cout << "coeffs: " << sp.coeffs << std::endl;

  Eigen::ArrayXXd x2{{
                         0,
                     },
                     {
                         1,
                     },
                     {
                         2,
                     },
                     {
                         5.0 / 2.0,
                     }};

  nuis::GaussRBFInterpolXd rbf(x2);
  rbf.build(y);
  std::cout << "coeffs: " << rbf.coeffs << std::endl;
}