#pragma once

#include "nuis/histframe/Binning.h"

#include "Eigen/Dense"

#include <string>
#include <vector>

namespace nuis {
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

  column_t add_column(std::string const &name, std::string const &label = "");
  column_t find_column_index(std::string const &name) const;

  Eigen::ArrayXd get_content(column_t col = 0,
                             bool divide_by_bin_sizes = false) const;
  Eigen::ArrayXd get_error(column_t col = 0,
                           bool divide_by_bin_sizes = false) const;

  Bins::BinId find_bin(std::vector<double> const &projections) const;
  // convenience for 1D histograms
  Bins::BinId find_bin(double proj) const;
  
  void fill_bin(Bins::BinId bini, double weight, column_t col = 0);

  void fill(std::vector<double> const &projections, double weight,
            column_t col = 0);

  // convenience for 1D histograms
  void fill(double proj, double weight, column_t col = 0);

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