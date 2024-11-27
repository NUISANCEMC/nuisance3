#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/eventframe/missing_datum.h"

#include "nuis/except.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace Eigen {
using ArrayXdRef = Ref<ArrayXd, 0, Stride<Dynamic, Dynamic>>;
using ArrayXdCRef = Ref<ArrayXd const, 0, Stride<Dynamic, Dynamic>> const;

} // namespace Eigen

namespace nuis {

DECLARE_NUISANCE_EXCEPT(InvalidFrameColumnName);

struct EventFrame {
  std::vector<std::string> column_names;
  Eigen::ArrayXXd table;
  size_t num_rows;

  using column_t = uint16_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  column_t find_column_index(std::string const &name) const;
  //as find but throws if named column doesn't exist
  column_t require_column_index(std::string const &name) const;

  Eigen::ArrayXdRef col(std::string const &cn);
  Eigen::ArrayXdCRef col(std::string const &cn) const;
  std::vector<Eigen::ArrayXdRef> cols(std::vector<std::string> const &cns);

  // Build a new event frame from a copy of the referenced rows.
  EventFrame topRows(size_t rows) const;
  EventFrame bottomRows(size_t rows) const;

  explicit operator bool() const { return table.rows(); }
};

struct EventFramePrinter {
  std::reference_wrapper<EventFrame const> fr;
  int max_rows;
  bool prettyprint;
  size_t max_col_width;

  explicit EventFramePrinter(EventFrame const &f, int mr = 20, bool pp = true,
                             size_t mcw = 12)
      : fr{f}, max_rows{mr}, prettyprint(pp), max_col_width{mcw} {}
};

std::ostream &operator<<(std::ostream &os, nuis::EventFrame const &);
std::ostream &operator<<(std::ostream &os, nuis::EventFramePrinter);

} // namespace nuis