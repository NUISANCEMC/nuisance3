#include "nuis/binning/Utility.h"
#include "nuis/binning/log_bin_edges.txx"

#include "nuis/log.txx"

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
              log_critical(
                  "[unique]: Tried to compare multi-dimensional binning for "
                  "equality with bins of unequal dimensionality.");
              throw MismatchedAxisCount();
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
    Binning::log_critical(
        "[bin_extents_overlap]: Tried to check for bin overlaps with bins "
        "of unequal dimensionality: {} != {}.",
        a.size(), b.size());
    throw MismatchedAxisCount();
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
        Binning::log_critical(
            "[project_to_unique_bins]: Tried to get dimension {} "
            "extent from binning with only {} dimensions.",
            proj_to_axis, bin.size());
        throw AxisOverflow();
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

std::vector<std::vector<double>>
get_bin_centers(std::vector<Binning::BinExtents> const &bins) {
  std::vector<std::vector<double>> bcs;
  for (auto const &bin : bins) {
    bcs.emplace_back();
    for (auto const &sext : bin) {
      bcs.back().push_back((sext.high + sext.low) / 2.0);
    }
  }

  return bcs;
}

std::vector<double>
get_bin_centers1D(std::vector<Binning::BinExtents> const &bins) {
  std::vector<double> bcs;
  auto bcs_all_axes = get_bin_centers(bins);
  for (auto const &bc : bcs_all_axes) {
    bcs.push_back(bc[0]);
  }
  return bcs;
}

std::vector<double> log10_spaced_edges(double start, double stop,
                                       size_t nbins) {
  return log_spaced_edges<10>(start, stop, nbins);
}
std::vector<double> ln_spaced_edges(double start, double stop, size_t nbins) {
  return log_spaced_edges(start, stop, nbins);
}

std::vector<double> uniform_width_edges(double start, double width, size_t nbins) {

  if (width <= 0) {
    Binning::log_critical("uniform_width_edges({0},{1},{2}) is invalid as width={1}.",
                          start, width, nbins);
    throw BinningNotIncreasing();
  }

  std::vector<double> edges = {start};
  for (size_t i = 0; i < nbins; ++i) {
    edges.push_back(edges.back() + width);
  }
  return edges;
}

std::vector<double> lin_spaced_edges(double start, double stop, size_t nbins) {
  if (start >= stop) {
    Binning::log_critical(
        "lin_spaced_edges({0},{1},{2}) is invalid as start={0} >= stop={1}.",
        start, stop, nbins);
    throw BinningNotIncreasing();
  }

  double width = (stop - start) / double(nbins);
  return uniform_width_edges(start, width, nbins);
}

std::vector<double>
cat_bin_edges(std::vector<std::vector<double>> const &edge_series) {
  std::vector<double> bin_edges;
  for (auto const &edges : edge_series) {
    std::copy(edges.begin(), edges.end(), std::back_inserter(bin_edges));
  }
  bin_edges.erase(bin_edges.begin(),
                  std::unique(bin_edges.begin(), bin_edges.end()));

  if (bin_edges.size() < 2) {
    Binning::log_critical("cat_bin_edges was passed too few bin edges, need at "
                          "least two, found {} unique edges.",
                          bin_edges.size());
    throw TooFewBinEdges();
  }

  for (size_t bi = 1; bi < bin_edges.size(); ++bi) {
    if (bin_edges[bi - 1] >= bin_edges[bi]) {
      Binning::log_critical("cat_bin_edges was passed unsorted bin edges. "
                            "After concatenation and removal of duplicates, "
                            "bin_edges[{}] = {} and bin_edges[{}] = {}.",
                            bi - 1, bin_edges[bi - 1], bi, bin_edges[bi]);
      throw BinningUnsorted();
    }
  }
  return bin_edges;
}

std::vector<Binning::BinExtents>
edges_to_extents(std::vector<double> const &bin_edges) {

  if (bin_edges.size() < 2) {
    Binning::log_critical(
        "edges_to_extents was passed too few bin edges, need at "
        "least two, found {} unique edges.",
        bin_edges.size());
    throw TooFewBinEdges();
  }

  std::vector<Binning::BinExtents> bins;
  for (size_t ei = 1; ei < bin_edges.size(); ++ei) {
    if (bin_edges[ei - 1] >= bin_edges[ei]) {
      Binning::log_critical(
          "[contiguous]: Bin edges are not unique and monotonically "
          "increasing. edge[{}] = {}, edge[{}] = {}.",
          ei, bin_edges[ei], ei - 1, bin_edges[ei - 1]);
      throw BinningUnsorted();
    }
    bins.emplace_back();
    bins.back().emplace_back(bin_edges[ei - 1], bin_edges[ei]);
  }
  return bins;
}

} // namespace nuis