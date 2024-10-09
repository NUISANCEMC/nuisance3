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

struct BinnedValuesBase : public nuis_named_log("HistFrame") {

  BinnedValuesBase(BinningPtr binop, std::string const &def_col_name = "mc",
                   std::string const &def_col_label = "");
  
  BinnedValuesBase(BinnedValuesBase const &other)
      : binning{other.binning}, column_info{other.column_info} {}

  BinnedValuesBase &operator=(BinnedValuesBase const &other) {
    binning = other.binning;
    column_info = other.column_info;
    return *this;
  }

  BinnedValuesBase() {}

  // --- types
  struct ColumnInfo {
    std::string name;
    std::string column_label;
  };

  using column_t = uint32_t;
  constexpr static column_t const npos = std::numeric_limits<column_t>::max();

  // independent variables
  BinningPtr binning;

  // dependent variables
  std::vector<ColumnInfo> column_info;

  column_t add_column(std::string const &name, std::string const &label = "");
  column_t find_column_index(std::string const &name) const;

  Binning::index_t find_bin(std::vector<double> const &projections) const;
  // convenience for 1D histograms
  Binning::index_t find_bin(double projection) const;

  // adjusts the shape of the data and uncertainty matrices so that
  // they are at least big enough to hold the binned values for
  // column_info.size() columns. Will not remove or overwrite any values.
  virtual void resize() = 0;

  virtual Eigen::ArrayXXdCRef get_bin_contents() const = 0;
  virtual Eigen::ArrayXXd get_bin_uncertainty() const = 0;
  virtual Eigen::ArrayXXd get_bin_uncertainty_squared() const = 0;
};

struct BinnedValues : public BinnedValuesBase {

  // --- data members
  struct column_view {
    Eigen::ArrayXdRef value;
    Eigen::ArrayXdRef error;
  };

  struct column_view_const {
    Eigen::ArrayXdCRef value;
    Eigen::ArrayXdCRef error;
  };

  // values and errors in bins
  Eigen::ArrayXXd values, errors;

  // --- constructors
  BinnedValues(BinningPtr binop, std::string const &def_col_name = "mc",
               std::string const &def_col_label = "")
      : BinnedValuesBase(binop, def_col_name, def_col_label) {
    resize();
  }

  BinnedValues(BinnedValues const &other)
      : BinnedValuesBase(other), values{other.values}, errors{other.errors} {};

  BinnedValues &operator=(BinnedValues const &other) {
    binning = other.binning;
    column_info = other.column_info;
    values = other.values;
    errors = other.errors;
    return *this;
  }

  BinnedValues() {};

  // --- methods

  column_view operator[](column_t col);
  column_view operator[](std::string const &name);

  column_view_const operator[](column_t col) const;
  column_view_const operator[](std::string const &name) const;

  // factory function for fillable HistFrames with the same binning as this
  // BinnedValues instance. If col == npos, copy all columns.
  HistFrame make_HistFrame(column_t col = 0) const;

  // adjusts the shape of BinnedValues::values and BinnedValues::errors so that
  // they are at least big enough to hold the binned values for
  // column_info.size(). Will not remove or overwrite data.
  void resize();

  Eigen::ArrayXXdCRef get_bin_contents() const { return values; }
  Eigen::ArrayXXd get_bin_uncertainty() const { return errors; }
  Eigen::ArrayXXd get_bin_uncertainty_squared() const {
    return errors.square();
  }
};

} // namespace nuis