#pragma once

#include "nuis/binning/Binning.h"
#include "nuis/binning/Exceptions.h"

#include "nuis/log.txx"

namespace nuis {

// base == 0 -> base e/natural log
template <uint base = 0> auto logbase(double v) {
  return (base == 0) ? std::log(v) : (std::log(v) / std::log(base));
}
template <uint base = 0> auto expbase(double v) {
  return (base == 0) ? std::exp(v) : std::exp(v * std::log(base));
}

template <uint base = 0>
std::vector<double> log_spaced_edges(double start, double stop, size_t nbins) {

  if (start <= 0) {
    Binning::log_critical(
        "log{}_spaced_edges({},{},{}) is invalid as start <= 0.",
        base == 0 ? "" : std::to_string(base), start, stop, nbins);
    throw InvalidBinEdgeForLogarithmicBinning();
  }

  if (start >= stop) {
    Binning::log_critical(
        "log{0}_spaced_edges({1},{2},{3}) is invalid as start={1} >= stop={2}.",
        base == 0 ? "" : std::to_string(base), start, stop, nbins);
    throw BinningNotIncreasing();
  }

  auto startl = logbase<base>(start);
  auto stopl = logbase<base>(stop);

  double lwidth = (stopl - startl) / double(nbins);

  std::vector<double> edges = {start};
  for (size_t i = 0; i < nbins; ++i) {
    edges.push_back(expbase<base>(startl + (i + 1) * lwidth));
  }

  Binning::log_trace(
      "[log{}_spaced_edges({},{},{})] startl={} stopl={} lwidth={}",
      base == 0 ? "" : std::to_string(base), start, stop, nbins, startl, stopl,
      lwidth);
  Binning::log_trace("  {}", edges);

  return edges;
}

} // namespace nuis