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
    std::string independent_axis_label;
  };

  std::vector<ColumnInfo> column_info;

  

  Eigen::ArrayXXd content, variance;
  size_t nfills;

  HistFrame(Bins::BinOp bindef);

  using column_t = uint32_t;

  column_t AddColumn(std::string const &name, std::string const &label = "");
  column_t GetColumnIndex(std::string const &name) const;

  HistColumn_View operator[](std::string const &name) const;

  HistColumn_View operator[](column_t const &colid) const;
  
  Bins::BinId FindBin(std::vector<double> const &projections) const;
  void Fill(std::vector<double> const &projections, double weight,
            column_t col = 1);
  void Fill(double proj, double weight, column_t col = 1);

  void ScaleColumn(double s, HistFrame::column_t col = 1,
                   bool divide_by_cell_area = false);
  void MultiplyColumn(Eigen::ArrayXd const &other, HistFrame::column_t col = 1);
  void DivideColumn(Eigen::ArrayXd const &other, HistFrame::column_t col = 1);
  double ColumnIntegral(HistFrame::column_t col = 1,
                        bool multiply_by_cell_area = false) const;
  void Reset();
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