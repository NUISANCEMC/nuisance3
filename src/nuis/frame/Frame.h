#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace nuis {

struct Frame {
  std::vector<std::string> ColumnNames;
  Eigen::MatrixXd Table;
  NormInfo norm_info;
};

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::Frame const &);