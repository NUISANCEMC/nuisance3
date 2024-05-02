#include "nuis/response/FramedResponse.h"

#include <iostream>

int main() {

  Eigen::Array4d x = {0, 1, 2, 5.0 / 2.0};
  Eigen::Array<double,1,4> y = {0, 1, 8, 9};

  nuis::NaturalCubicFrameSpline<4> sp(x);
  sp.build(y);

  std::cout << (sp.coeffs * 11) << std::endl;
}