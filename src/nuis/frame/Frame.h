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

struct FramePrinter {
  std::reference_wrapper<Frame const> fr;
  int max_rows;
  bool prettyprint;
  size_t max_col_width;

  explicit FramePrinter(Frame const &f, int mr = 20, bool pp = true,
                        size_t mcw = 12)
      : fr{f}, max_rows{mr}, prettyprint(pp), max_col_width{mcw} {}
};

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::Frame const &);
std::ostream &operator<<(std::ostream &os, nuis::FramePrinter);