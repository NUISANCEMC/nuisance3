#pragma once

#include "nuis/except.h"

#include "nuis/log.h"

#include "yaml-cpp/yaml.h"

#include "Eigen/Dense"

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace nuis {

NEW_NUISANCE_EXCEPT(BinningNotIncreasing);
NEW_NUISANCE_EXCEPT(UnbinnableNumber);
NEW_NUISANCE_EXCEPT(InvalidBinEdgForLogarithmicBinning);
NEW_NUISANCE_EXCEPT(TooFewProjectionsForBinning);
NEW_NUISANCE_EXCEPT(BinningUnsorted);
NEW_NUISANCE_EXCEPT(BinningHasOverlaps);
NEW_NUISANCE_EXCEPT(BinningNotUnique);
NEW_NUISANCE_EXCEPT(MismatchedAxisCount);
NEW_NUISANCE_EXCEPT(AxisOverflow);
NEW_NUISANCE_EXCEPT(CatastrophicBinningFailure);

struct Binning : public nuis_named_log("Binning") {

  //--- types
  using Index = uint32_t;

  struct SingleExtent {
    double min;
    double max;
    SingleExtent(double mi = 0, double ma = 0) : min(mi), max(ma) {}

    double width() const { return (max - min); }

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

  std::function<Index(std::vector<double> const &)> find_bin;

  // convenience find_bintor-like overloads for calling Binning::find_bin
  Index operator()(std::vector<double> const &) const;
  Index operator()(double) const;

  //--- member find_bintions

  // Get the size for every bin.
  // The size will depend on the dimensionality of the binning: for 1D binning
  // it will correspond to the bin width, for 2D, the bin area, for 3D the bin
  // volume, etc...
  Eigen::ArrayXd bin_sizes() const;

  //--- static find_bintions

  static Binning lin_space(double min, double max, size_t nbins,
                           std::string const &label = "");
  static Binning lin_spaceND(std::vector<std::tuple<double, double, size_t>>,
                             std::vector<std::string> = {});

  static Binning log_space(double min, double max, size_t nbins,
                           std::string const &label = "");
  static Binning log10_space(double min, double max, size_t nbins,
                             std::string const &label = "");

  // bin edges must be unique and monotonically increasing
  static Binning contiguous(std::vector<double> const &edges,
                            std::string const &label = "");

  // extents must be unique and non-overlapping
  static Binning from_extents(std::vector<BinExtents> extents,
                              std::vector<std::string> const &labels = {});

  static Binning product(std::vector<Binning> const &ops);
};

// sort bins based on extent in each dimension in decreasing dimension order
// so that neighbouring bins are neighbouring in the first axis.
bool operator<(Binning::BinExtents const &, Binning::BinExtents const &);

bool operator<(Binning::SingleExtent const &, Binning::SingleExtent const &);
bool operator==(Binning::SingleExtent const &, Binning::SingleExtent const &);

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::Binning::SingleExtent const &);
std::ostream &operator<<(std::ostream &os, nuis::Binning::BinExtents const &);
std::ostream &operator<<(std::ostream &os,
                         std::vector<nuis::Binning::BinExtents> const &);
std::ostream &operator<<(std::ostream &os, nuis::Binning const &);