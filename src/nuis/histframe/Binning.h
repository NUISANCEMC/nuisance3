#pragma once

#include "yaml-cpp/yaml.h"

#include <cstdint>
#include <functional>
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

    bool operator==(extent const &other) {
      return (min == other.min) && (max == other.max);
    }
    bool operator<(extent const &other) { return min < other.min; }
  };
  // BinExtents[i] are the N extents of bin i.
  std::vector<std::vector<extent>> extents;
};

struct BinOp {
  BinningInfo bin_info;
  BinningF bin_func;
};

BinOp LinSpace(size_t nbins, double min, double max,
               std::string const &label = "");
BinOp LinSpaceND(std::vector<std::tuple<size_t, double, double>>,
                 std::vector<std::string> = {});

} // namespace Bins
} // namespace nuis