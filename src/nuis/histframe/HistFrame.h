#pragma once

#include "nuis/binning/Binning.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/log.h"

#include "Eigen/Dense"

#include <iostream>
#include <string>
#include <vector>

namespace nuis {

struct HistFrame : public BinnedValuesBase {

  // --- data members

  // sum of weights and variance in bins
  Eigen::ArrayXXd sumweights, variances;

  size_t num_fills;

  // --- constructors

  HistFrame(BinningPtr binop, std::string const &def_col_name = "mc",
            std::string const &def_col_label = "")
      : BinnedValuesBase(binop, def_col_name, def_col_label) {
    reset();
  }
  HistFrame(){};

  struct column_view {
    Eigen::ArrayXdRef count;
    Eigen::ArrayXdRef variance;
  };

  struct column_view_const {
    Eigen::ArrayXdCRef count;
    Eigen::ArrayXdCRef variance;
  };

  column_view operator[](column_t col);
  column_view operator[](std::string const &name);

  column_view_const operator[](column_t col) const;
  column_view_const operator[](std::string const &name) const;

  void fill(std::vector<double> const &projections, double weight,
            column_t col = 0);
  // A semantically meaningful helper function for passing a selection
  // integer as the first projection axis
  void fill_with_selection(int sel_int, std::vector<double> const &projections,
                           double weight, column_t col = 0);

  // convenience for 1D histograms
  void fill(double projection, double weight, column_t col = 0);
  void fill_with_selection(int sel_int, double projection, double weight,
                           column_t col = 0);

  void fill_bin(Binning::index_t bini, double weight, column_t col = 0);

  BinnedValues finalise(bool divide_by_bin_sizes = true) const;

  void reset();

  // adjusts the shape of BinnedValues::values and BinnedValues::errors so that
  // they are at least big enough to hold the binned values for
  // column_info.size(). Will not remove or overwrite data.
  void resize();

  Eigen::ArrayXXdCRef get_bin_contents() const { return sumweights; }
  Eigen::ArrayXXd get_bin_uncertainty() const { return variances.sqrt(); }
  Eigen::ArrayXXd get_bin_uncertainty_squared() const { return variances; }
};

} // namespace nuis