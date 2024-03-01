#pragma once

#include "yaml-cpp/yaml.h"

#include "Eigen/Dense"

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace nuis {
namespace Bins {

using BinId = uint32_t;
constexpr BinId npos = std::numeric_limits<BinId>::max();

using BinningF = std::function<BinId(std::vector<double> const &)>;

struct BinningInfo {
  YAML::Node yaml;

  std::vector<std::string> axis_labels;

  struct extent {
    double min;
    double max;

    double width() const { return (max - min); }

    bool operator==(extent const &other) const {
      return (min == other.min) && (max == other.max);
    }
    bool operator<(extent const &other) const { return min < other.min; }
  };
  // BinExtents[i] are the N extents of bin i.
  std::vector<std::vector<extent>> extents;

  // Get the size for each bin.
  // The size will depend on the dimensionality of the binning: for 1D binning
  // it will correspond to the bin width, for 2D, the bin area, for 3D the bin
  // volume, etc...
  Eigen::ArrayXd bin_sizes() const;
};

struct BinOp {
  BinningInfo bin_info;
  BinningF bin_func;
};

BinOp combine(std::vector<BinOp> const &ops);

BinOp log_space(size_t nbins, double min, double max,
                std::string const &label = "");
BinOp log10_space(size_t nbins, double min, double max,
                  std::string const &label = "");

BinOp lin_space(size_t nbins, double min, double max,
                std::string const &label = "");
BinOp lin_spaceND(std::vector<std::tuple<size_t, double, double>>,
                  std::vector<std::string> = {});

BinOp from_extents1D(std::vector<BinningInfo::extent> extents,
                     std::string const &label = "");

} // namespace Bins
} // namespace nuis

std::ostream &operator<<(std::ostream &os,
                         nuis::Bins::BinningInfo::extent const &);
std::ostream &operator<<(std::ostream &os, nuis::Bins::BinningInfo const &);