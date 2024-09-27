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

  HistFrame(HistFrame const &other)
      : BinnedValuesBase(other), sumweights{other.sumweights},
        variances{other.variances}, num_fills{other.num_fills} {};

  HistFrame &operator=(HistFrame const &other) {
    binning = other.binning;
    column_info = other.column_info;
    sumweights = other.sumweights;
    variances = other.variances;
    num_fills = other.num_fills;
    return *this;
  }

  HistFrame() {};

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

  void fill(std::vector<double> const &projections, double weight);
  void fill_column(std::vector<double> const &projections, double weight,
                   column_t col);
  void fill_if(bool selected, std::vector<double> const &projections,
               double weight);
  void fill_column_if(bool selected, std::vector<double> const &projections,
                      double weight, column_t col);

  // convenience for 1D histograms
  void fill(double projection, double weight);
  void fill_column(double projection, double weight, column_t col);
  void fill_if(bool selected, double projection, double weight);
  void fill_column_if(bool selected, double projection, double weight,
                      column_t col);

  void fill_bin(Binning::index_t bini, double weight, column_t col);

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