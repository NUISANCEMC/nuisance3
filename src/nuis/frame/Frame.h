#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "Eigen/Dense"

#include <vector>
#include <string>

namespace nuis {

struct Frame {
  std::vector<std::string> ColumnNames;
  Eigen::MatrixXd Table;
  NormInfo norm_info; 
};

}