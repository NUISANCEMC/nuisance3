#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/frame/missing_datum.h"

#include "nuis/except.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace Eigen {
using ArrayXdRef = Ref<ArrayXd, 0, Stride<Dynamic, Dynamic>>;
} // namespace Eigen

namespace nuis {

NEW_NUISANCE_EXCEPT(InvalidFrameColumnName);

struct Frame {
  std::vector<std::string> column_names;
  Eigen::ArrayXXd table;
  NormInfo norm_info;

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  column_t find_column_index(std::string const &name) const;

  Eigen::ArrayXdRef col(std::string const &cn);
  std::vector<Eigen::ArrayXdRef> cols(std::vector<std::string> const &cns);
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