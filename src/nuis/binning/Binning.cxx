#include "nuis/binning/Binning.h"
#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "nuis/log.txx"

#include "fmt/ranges.h"

#include <cmath>

namespace nuis {

Binning::index_t Binning::find_bin(std::vector<double> const &x) const {
  return binning_function(x);
}
Binning::index_t Binning::find_bin(double x) const {
  static std::vector<double> vect = {0};
  vect[0] = x;
  return binning_function(vect);
}

Eigen::ArrayXd Binning::bin_sizes(size_t cols) const {
  Eigen::ArrayXd bin_sizes = Eigen::ArrayXd::Zero(bins.size()*cols);
  size_t i = 0;
  for(size_t j = 0; j < cols; ++j) {
    for (auto const &bin : bins) {
      bin_sizes[i] = 1;
      for (auto const &ext : bin) {
        bin_sizes[i] *= ext.width();
      }
      i++;
    }
  }
  return bin_sizes;
}

size_t Binning::number_of_axes() const {
  for (auto const &bin : bins) {
    return bin.size();
  }
  return 0;
}

bool operator<(Binning::BinExtents const &a, Binning::BinExtents const &b) {
  if (a.size() != b.size()) {
    log_critical(
        "[operator<(Binning::BinExtents const &a, Binning::BinExtents const "
        "&b)]: Tried to sort multi-dimensional binning with "
        "bins of unequal dimensionality: {} != {}",
        a.size(), b.size());
    throw MismatchedAxisCount();
  }
  for (size_t i = a.size(); i > 0; i--) {
    if (!(a[i - 1] == b[i - 1])) {
      return a[i - 1] < b[i - 1];
    }
  }
  // if we've got here the bin is identical in all dimensions
  return false;
}

std::ostream &operator<<(std::ostream &os, Binning::BinExtents const &bext) {
  os << "[";
  for (size_t j = 0; j < bext.size(); ++j) {
    os << bext[j] << ((j + 1) == bext.size() ? "]" : ", ");
  }
  return os;
}
std::ostream &operator<<(std::ostream &os,
                         std::vector<Binning::BinExtents> const &bins) {
  os << "[" << std::endl;
  for (size_t i = 0; i < bins.size(); ++i) {
    os << "  " << i << ": " << bins[i] << std::endl;
  }
  return os << "]" << std::endl;
}

std::ostream &operator<<(std::ostream &os, Binning const &bi) {
  os << fmt::format("Axis lables: {}\nBins: ", bi.axis_labels);
  return os << bi.bins;
}

} // namespace nuis