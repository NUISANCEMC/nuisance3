#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace nuis {

struct Frame {
  std::vector<std::string> column_names;
  Eigen::ArrayXXd table;
  NormInfo norm_info;

  constexpr static double const missing_datum = 0xdeadbeef;

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  column_t find_column_index(std::string const &name) const;

  // get a copy of a column, if you want to set a column, access the table
  // directly
  Eigen::ArrayXd col(std::string const &cn) const;
  Eigen::ArrayXXd cols(std::vector<std::string> const &cns) const;
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