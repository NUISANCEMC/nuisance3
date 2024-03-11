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
} // namespace Eigen

namespace nuis {

struct HistFrame : nuis_named_log("HistFrame") {

  struct ColumnInfo {
    std::string name;
    std::string dependent_axis_label;
  };

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  constexpr static double const missing_datum = 0xdeadbeef;

  struct column_view {
    Eigen::ArrayXdRef content;
    Eigen::ArrayXdRef variance;
    Eigen::ArrayXdRef const bin_weights;
  };

  struct column_valerr {
    Eigen::ArrayXd values;
    Eigen::ArrayXd errors;
  };

  std::vector<ColumnInfo> column_info;

  BinningPtr binning;
  Eigen::ArrayXd bin_weights;

  Eigen::ArrayXXd contents, variance;

  size_t nfills;

  HistFrame(BinningPtr binop, std::string const &def_col_name = "mc",
            std::string const &def_col_label = "");

  HistFrame(){};

  column_t add_column(std::string const &name, std::string const &label = "");
  column_t find_column_index(std::string const &name) const;

  Binning::index_t find_bin(std::vector<double> const &projections) const;

  Eigen::ArrayXd get_values(column_t col = 0) const;
  Eigen::ArrayXd get_errors(column_t col = 0) const;
  column_valerr get_column(column_t col = 0) const;

  column_view operator[](column_t col);
  column_view operator[](std::string const &name);

  void fill(std::vector<double> const &projections, double weight,
            column_t col = 0);
  // A semantically meaningful helper function for passing a selection
  // integer as the first projection axis
  void fill_with_selection(int sel_int, std::vector<double> const &projections,
                           double weight, column_t col = 0);

  // convenience for 1D histograms
  Binning::index_t find_bin(double projection) const;
  void fill(double projection, double weight, column_t col = 0);
  void fill_with_selection(int sel_int, double projection, double weight,
                           column_t col = 0);

  void fill_bin(Binning::index_t bini, double weight, column_t col = 0);

  // when
  void set_value_is_content_density();
  void set_value_is_content();

  void reset();
};

struct HistFramePrinter {
  std::reference_wrapper<HistFrame const> fr;
  int max_rows;
  size_t max_col_width;

  explicit HistFramePrinter(HistFrame const &f, int mr = 20, size_t mcw = 12)
      : fr{f}, max_rows{mr}, max_col_width{mcw} {}
};

std::ostream &operator<<(std::ostream &os, nuis::HistFrame const &);
std::ostream &operator<<(std::ostream &os, nuis::HistFramePrinter);

} // namespace nuis