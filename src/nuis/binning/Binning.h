#pragma once

#include "nuis/binning/SingleExtent.h"

#include "nuis/except.h"
#include "nuis/log.h"

#include "yaml-cpp/yaml.h"

#include "Eigen/Dense"

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace nuis {

struct Binning;

using BinningPtr = std::shared_ptr<Binning>;

struct Binning : public nuis_named_log("Binning") {

  //--- types
  using index_t = uint32_t;
  using BinExtents = std::vector<SingleExtent>;

  //--- constants
  static constexpr index_t npos = std::numeric_limits<index_t>::max();

  //--- data members
  std::vector<std::string> axis_labels;

  // extents[i] are the N SingleExtents of bin i.
  std::vector<BinExtents> bins;

  std::function<index_t(std::vector<double> const &)> binning_function;

  // convenience functor-like overloads for calling Binning::binning_function
  index_t find_bin(std::vector<double> const &) const;
  index_t find_bin(double) const;

  //--- member functions

  // Get the size for every bin.
  // The size will depend on the dimensionality of the binning: for 1D binning
  // it will correspond to the bin width, for 2D, the bin area, for 3D the bin
  // volume, etc...
  Eigen::ArrayXd bin_sizes() const;

  size_t number_of_axes() const;

  //--- static factory functions

  static BinningPtr lin_space(double start, double stop, size_t nbins,
                              std::string const &label = "");
  static BinningPtr lin_spaceND(std::vector<std::tuple<double, double, size_t>>,
                                std::vector<std::string> = {});

  static BinningPtr ln_space(double start, double stop, size_t nbins,
                             std::string const &label = "");
  static BinningPtr log10_space(double start, double stop, size_t nbins,
                                std::string const &label = "");

  // bin edges must be unique and monotonically increasing
  static BinningPtr contiguous(std::vector<double> const &edges,
                               std::string const &label = "");

  // extents must be unique and non-overlapping
  static BinningPtr from_extents(std::vector<BinExtents> extents,
                                 std::vector<std::string> const &labels = {});

  static BinningPtr product(std::vector<BinningPtr> ops);
};

// sort bins based on extent in each dimension in decreasing dimension order
// so that neighbouring bins are neighbouring in the first axis.
bool operator<(Binning::BinExtents const &, Binning::BinExtents const &);

std::ostream &operator<<(std::ostream &os, Binning::BinExtents const &);
std::ostream &operator<<(std::ostream &os,
                         std::vector<Binning::BinExtents> const &);
std::ostream &operator<<(std::ostream &os, Binning const &);

} // namespace nuis