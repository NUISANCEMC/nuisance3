#include "nuis/histframe/BinningUtility.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

#include <cmath>

namespace nuis {

std::vector<Binning::BinExtents> unique(std::vector<Binning::BinExtents> bins) {
  std::stable_sort(bins.begin(), bins.end());
  bins.erase(
      std::unique(
          bins.begin(), bins.end(),
          [](Binning::BinExtents const &a, Binning::BinExtents const &b) {
            if (a.size() != b.size()) {
              spdlog::critical(
                  "[unique]: Tried to compare multi-dimensional binning for "
                  "equality with bins of unequal dimensionality.");
              abort();
            }
            for (size_t i = 0; i < a.size(); ++i) {
              if (!(a[i] == b[i])) {
                return false;
              }
            }
            return true;
          }),
      bins.end());
  return bins;
}

bool bins_overlap(Binning::BinExtents const &a, Binning::BinExtents const &b) {
  if (a.size() != b.size()) {
    spdlog::critical(
        "[bin_extents_overlap]: Tried to check for bin overlaps with bins "
        "of unequal dimensionality: {} != {}.",
        a.size(), b.size());
    abort();
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (!a[i].overlaps(b[i])) { // must overlap in all dimensions to be consider
                                // a bin overlap
      return false;
    }
  }
  return true;
}

std::vector<Binning::BinExtents>
project_to_unique_bins(std::vector<Binning::BinExtents> const &bins,
                       std::vector<size_t> const &proj_to_axes) {

  if (!proj_to_axes.size()) {
    return unique(bins);
  }

  std::vector<Binning::BinExtents> proj_bins;
  for (auto const &bin : bins) {
    proj_bins.emplace_back();
    for (auto proj_to_axis : proj_to_axes) {

      if (bin.size() <= proj_to_axis) {
        spdlog::critical("[project_to_unique_bins]: Tried to get dimension {} "
                         "extent from binning with only {} dimensions.",
                         proj_to_axis, bin.size());
        abort();
      }

      proj_bins.back().push_back(bin[proj_to_axis]);
    }
  }

  return unique(proj_bins);
}

bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          std::vector<size_t> const &proj_to_axes) {
  auto proj_bins = project_to_unique_bins(bins, proj_to_axes);

  for (size_t i = 0; i < proj_bins.size(); ++i) {
    for (size_t j = i + 1; j < proj_bins.size();
         ++j) { // check every bin against every other bin
      if (bins_overlap(proj_bins[i], proj_bins[j])) {
        return true;
      }
    }
  }

  return false;
}

bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          size_t proj_to_axis) {
  return binning_has_overlaps(bins, std::vector<size_t>{
                                        proj_to_axis,
                                    });
}

} // namespace nuis