#pragma once

#include "nuis/histframe/Binning.h"

#include "Eigen/Dense"

#include <string>
#include <vector>
#include <iostream>

namespace nuis {

struct HistColumn_View {
  Eigen::ArrayXd content;
  Eigen::ArrayXd variance;
  Eigen::ArrayXd binwidth;

  // Need bin width options (don't love this)
  // Can we have some change with a visitor/reduction?
  Eigen::ArrayXd getcv(bool get_by_bin_width = false) {
    if (get_by_bin_width) return content / binwidth;
    return content;
  }

  void setcv(const Eigen::ArrayXd& incol, bool is_by_bin_width = false) {
    if (is_by_bin_width) content = incol * binwidth;
    content = incol;
  }

  Eigen::ArrayXd geter(bool get_by_bin_width = false) {
    if (get_by_bin_width) return variance / binwidth;
    return variance;
  }

  void seter(const Eigen::ArrayXd& incol, bool is_by_bin_width = false) {
    if (is_by_bin_width) variance = incol * binwidth;
    variance = incol;
  }
};

struct HistFrame {

  Bins::BinOp binning;

  struct ColumnInfo {
    std::string name;
    std::string dependent_axis_label;
  };

  std::vector<ColumnInfo> column_info;

  Eigen::ArrayXXd contents, variance;
  size_t nfills;

  HistFrame(Bins::BinOp bindef, std::string const &def_col_name = "mc",
            std::string const &def_col_label = "");

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  column_t add_column(std::string const &name, std::string const &label = "");
  column_t find_column_index(std::string const &name) const;

  Eigen::ArrayXd get_content(column_t col = 0,
                             bool divide_by_bin_sizes = false) const;
  Eigen::ArrayXd get_error(column_t col = 0,
                           bool divide_by_bin_sizes = false) const;

  HistColumn_View operator[](std::string const &name) const;
  HistColumn_View operator[](column_t const& colid) const;

  

  Bins::BinId find_bin(std::vector<double> const &projections) const;
  void fill(std::vector<double> const &projections, double weight,
            column_t col = 0);
  // A semantically meaningful helper function for passing a selection integer
  // as the first projection axis
  void fill_with_selection(int sel_int, std::vector<double> const &projections,
                           double weight, column_t col = 0);

  // convenience for 1D histograms
  Bins::BinId find_bin(double projection) const;
  void fill(double projection, double weight, column_t col = 0);
  void fill_with_selection(int sel_int, double projection, double weight,
                           column_t col = 0);

  void fill_bin(Bins::BinId bini, double weight, column_t col = 0);

  void reset();
};

struct HistFramePrinter {
  std::reference_wrapper<HistFrame const> fr;
  std::string format;
  int max_rows;
  size_t max_col_width;

  explicit HistFramePrinter(HistFrame const &f, std::string fmt = "table",
                            int mr = 20, size_t mcw = 12)
      : fr{f}, format{fmt}, max_rows{mr}, max_col_width{mcw} {}
};

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::HistFrame const &);
std::ostream &operator<<(std::ostream &os, nuis::HistFramePrinter);