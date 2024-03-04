#pragma once

#include "yaml-cpp/yaml.h"

#include "Eigen/Dense"

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace nuis {
struct Binning {

  //--- types
  using Index = uint32_t;

  struct SingleExtent {
    double min;
    double max;

    double width() const { return (max - min); }

    bool operator==(SingleExtent const &other) const {
      return (min == other.min) && (max == other.max);
    }
    bool operator<(SingleExtent const &other) const { return min < other.min; }

    bool overlaps(SingleExtent const &other) const {
      if (other.min < min) { // overlaps if other.max > min
        return (other.max > min);
      }
      if (other.max > max) { // overlaps if other.max > min
        return (other.min < max);
      }
      return true; // other is completely inside this
    }

    bool contains(double x) const { return (x >= min) && (x < max); }
  };

  using BinExtents = std::vector<SingleExtent>;

  //--- constants
  static constexpr Index npos = std::numeric_limits<Index>::max();

  //--- data members
  std::vector<std::string> axis_labels;

  // extents[i] are the N SingleExtents of bin i.
  std::vector<BinExtents> bins;

  std::function<Index(std::vector<double> const &)> func;

  //--- member functions

  // Get the size for every bin.
  // The size will depend on the dimensionality of the binning: for 1D binning
  // it will correspond to the bin width, for 2D, the bin area, for 3D the bin
  // volume, etc...
  Eigen::ArrayXd bin_sizes() const;

  //--- static functions

  static Binning lin_space(size_t nbins, double min, double max,
                           std::string const &label = "");
  static Binning lin_spaceND(std::vector<std::tuple<size_t, double, double>>,
                             std::vector<std::string> = {});

  static Binning log_space(size_t nbins, double min, double max,
                           std::string const &label = "");
  static Binning log10_space(size_t nbins, double min, double max,
                             std::string const &label = "");

  // bin edges must be unique and monotonically increasing
  static Binning contiguous(std::vector<double> const &edges,
                            std::string const &label = "");

  // extents must be unique and non-overlapping
  static Binning from_extents(std::vector<BinExtents> extents,
                              std::vector<std::string> const &labels = {});

  static Binning product(std::vector<Binning> const &ops);
};

} // namespace nuis