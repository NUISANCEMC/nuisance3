#pragma once

#include "nuis/binning/Binning.h"

#include "nuis/log.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace Eigen {
using ArrayXXdRef = Ref<ArrayXXd, 0, Stride<Dynamic, Dynamic>>;
using ArrayXdRef = Ref<ArrayXd, 0, Stride<Dynamic, Dynamic>>;
using ArrayXXdCRef = Ref<ArrayXXd const, 0, Stride<Dynamic, Dynamic>> const;
using ArrayXdCRef = Ref<ArrayXd const, 0, Stride<Dynamic, Dynamic>> const;
} // namespace Eigen

namespace nuis {

class HistFrame;

struct BinnedValues : nuis_named_log("HistFrame") {

  // --- types
  struct ColumnInfo {
    std::string name;
    std::string dependent_axis_label;
  };

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  struct column_view {
    Eigen::ArrayXdRef value;
    Eigen::ArrayXdRef error;
  };

  struct column_view_const {
    Eigen::ArrayXdCRef value;
    Eigen::ArrayXdCRef error;
  };

  // --- data members

  // independent variables
  BinningPtr binning;

  // dependent variables
  std::vector<ColumnInfo> column_info;

  // values and errors in bins
  Eigen::ArrayXXd values, errors;

  // --- methods
  BinnedValues(BinningPtr binop, std::string const &def_col_name = "mc",
               std::string const &def_col_label = "");

  BinnedValues(){};

  column_t add_column(std::string const &name, std::string const &label = "");
  column_t find_column_index(std::string const &name) const;

  Binning::index_t find_bin(std::vector<double> const &projections) const;
  // convenience for 1D histograms
  Binning::index_t find_bin(double projection) const;

  column_view operator[](column_t col);
  column_view operator[](std::string const &name);

  column_view_const operator[](column_t col) const;
  column_view_const operator[](std::string const &name) const;

  // factory function for fillable HistFrames with the same binning as this
  // BinnedValues instance. If col == npos, copy all columns.
  HistFrame make_HistFrame(column_t col = 0) const;
};

struct BinnedValuesPrinter {
  std::reference_wrapper<BinnedValues const> fr;
  int max_rows;
  size_t max_col_width;

  explicit BinnedValuesPrinter(BinnedValues const &f, int mr = 20,
                               size_t mcw = 12)
      : fr{f}, max_rows{mr}, max_col_width{mcw} {}
};

std::ostream &operator<<(std::ostream &os, nuis::BinnedValues const &);
std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesPrinter);

} // namespace nuis