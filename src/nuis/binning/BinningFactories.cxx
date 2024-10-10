#include "nuis/binning/Binning.h"

#include "nuis/binning/log_bin_edges.txx"
#include "nuis/binning/utility.h"

#include "spdlog/fmt/bundled/ranges.h"

namespace nuis {

BinningPtr Binning::lin_space(double start, double stop, size_t nbins,
                              std::string const &label) {

  if (start >= stop) {
    Binning::log_critical(
        "lin_space({0},{1},{2}) is invalid as start={0} >= stop={1}.", start,
        stop, nbins);
    throw BinningNotIncreasing();
  }

  double width = (stop - start) / double(nbins);

  std::vector<Binning::BinExtents> bins =
      edges_to_extents(uniform_width_edges(start, width, nbins));

  BinningPtr bin_info = std::make_shared<Binning>();
  bin_info->axis_labels.push_back(label);

  // be careful not to capture bin_info or you will create a circular reference
  // for the shared_ptr
  bin_info->binning_function = [=](std::vector<double> const &x) -> index_t {
    NUIS_LOG_TRACE(
        "[lin_space({},{},{}).binning_function] x.size() = {}, x[0] = {}",
        start, stop, nbins, x.size(), x.size() ? x[0] : 0xdeadbeef);
    if (x.size() < 1) {
      log_warn("[lin_space({},{},{}).binning_function] was passed an empty "
               "projection "
               "vector. Returning npos. Compile with "
               "CMAKE_BUILD_TYPE=Debug to make this an exception.",
               start, stop, nbins);
#ifndef NUIS_NDEBUG
      throw TooFewProjectionsForBinning();
#endif
      return npos;
    }

    if ((x[0] != 0) && !std::isnormal(x[0])) {
      log_warn("[lin_space({},{},{}).binning_function] was passed an "
               "abnornmal number = {}. Returning npos. Compile with "
               "CMAKE_BUILD_TYPE=Debug to make this an exception.",
               start, stop, nbins, x[0]);
#ifndef NUIS_NDEBUG
      throw UnbinnableNumber();
#endif
      return Binning::npos;
    }

    if (x[0] >= stop) {
      NUIS_LOG_TRACE("[lin_space({},{},{}).binning_function]: {} above extent. "
                     "Returning npos.",
                     start, stop, nbins, x[0]);
      return npos;
    }

    if (x[0] < start) {
      NUIS_LOG_TRACE("[lin_space({},{},{}).binning_function]: {} below extent. "
                     "Returning npos.",
                     start, stop, nbins, x[0]);
      return npos;
    }

    index_t bin = std::floor((x[0] - start) / width);
    NUIS_LOG_TRACE(
        "[lin_space({},{},{}).binning_function] Found bin: {}, v = {} "
        "is in ({} -- {})",
        start, stop, nbins, bin, x[0], bins[bin][0].low, bins[bin][0].high);
    return bin;
  };

  bin_info->bins = bins;
  return bin_info;
}

BinningPtr
Binning::lin_spaceND(std::vector<std::tuple<double, double, size_t>> axes,
                     std::vector<std::string> labels) {

  size_t nax = axes.size();
  std::vector<size_t> nbins_in_slice = {1};
  std::vector<double> axis_bin_widths;

  size_t nbins = 1;
  for (size_t ax_i = 0; ax_i < nax; ++ax_i) {

    auto const &ax_start = std::get<0>(axes[ax_i]);
    auto const &ax_stop = std::get<1>(axes[ax_i]);
    auto const &ax_nbins = std::get<2>(axes[ax_i]);

    if (ax_start >= ax_stop) {
      log_critical(
          "lin_spaceND({}) is invalid along axis {} as start={} >= stop={}.",
          axes, ax_i, ax_start, ax_stop);
      throw BinningNotIncreasing();
    }

    axis_bin_widths.push_back((ax_stop - ax_start) / double(ax_nbins));
    nbins *= ax_nbins;
    nbins_in_slice.push_back(nbins);
  }

  BinningPtr bin_info = std::make_shared<Binning>();
  for (size_t i = 0; i < nax; ++i) {
    bin_info->axis_labels.push_back(labels.size() > i ? labels[i] : "");
  }

  std::vector<Binning::BinExtents> bins;
  for (size_t bin_i = 0; bin_i < nbins; ++bin_i) {
    bins.emplace_back(nax, SingleExtent{});

    size_t bin_remainder = bin_i;
    for (int ax_i = int(nax - 1); ax_i >= 0; --ax_i) {

      auto const &ax_start = std::get<0>(axes[ax_i]);

      index_t bin_along_ax = (bin_remainder / nbins_in_slice[ax_i]);
      bin_remainder = bin_remainder % nbins_in_slice[ax_i];

      bins.back()[ax_i] = {ax_start + (bin_along_ax * axis_bin_widths[ax_i]),
                           ax_start +
                               ((bin_along_ax + 1) * axis_bin_widths[ax_i])};
    }
  }

  // be careful not to capture bin_info or you will create a circular reference
  // for the shared_ptr
  bin_info->binning_function = [=](std::vector<double> const &x) -> index_t {
    NUIS_LOG_TRACE(
        "[lin_spaceND({}).binning_function] x.size() = {}/{} axes, x = {}",
        axes, x.size(), nax, x);

    if (x.size() < nax) {
      log_warn(
          "[lin_spaceND({}).binning_function] was passed too few projections. "
          "x.size() = "
          "{} < nax = {}. Returning npos. Compile with "
          "CMAKE_BUILD_TYPE=Debug to make this an exception.",
          axes, x.size(), nax);
#ifndef NUIS_NDEBUG
      throw TooFewProjectionsForBinning();
#endif
      return npos;
    }

    index_t gbin = 0;
    for (size_t ax_i = 0; ax_i < nax; ++ax_i) {

      if ((x[ax_i] != 0) && !std::isnormal(x[ax_i])) {
        log_warn(
            "[lin_spaceND({}).binning_function] was passed an "
            "abnornmal number = {} on axis {}. Returning npos. Compile with "
            "CMAKE_BUILD_TYPE=Debug to make this an exception.",
            axes, x[ax_i], ax_i);
#ifndef NUIS_NDEBUG
        throw UnbinnableNumber();
#endif
        return Binning::npos;
      }

      auto const &ax_start = std::get<0>(axes[ax_i]);
      auto const &ax_stop = std::get<1>(axes[ax_i]);

      if (x[ax_i] >= ax_stop) {
        NUIS_LOG_TRACE(
            "[lin_spaceND({}).binning_function] above extent on axis {}. "
            "Returning npos.",
            axes, ax_i);
        return npos;
      }

      if (x[ax_i] < ax_start) {
        NUIS_LOG_TRACE(
            "[lin_spaceND({}).binning_function] below extent on axis {}. "
            "Returning npos.",
            axes, ax_i);
        return npos;
      }

      index_t dimbin = std::floor((x[ax_i] - ax_start) / axis_bin_widths[ax_i]);

      NUIS_LOG_TRACE(
          "[lin_spaceND({}).binning_function] Found bin[{}]: {}, v = {} "
          "is in ({} -- {})",
          axes, ax_i, dimbin, x[ax_i], bins[dimbin][ax_i].low,
          bins[dimbin][ax_i].high);

      gbin += dimbin * nbins_in_slice[ax_i];
      NUIS_LOG_TRACE(
          "[lin_spaceND({}).binning_function] gbin after {} axes: {}", axes,
          ax_i, gbin);
    }

    NUIS_LOG_TRACE(
        "[lin_spaceND({}).binning_function] returning gbin {} for x = {}", axes,
        gbin, x);

    return gbin;
  };
  bin_info->bins = bins;
  return bin_info;
}

template <uint base>
BinningPtr log_space_impl(double start, double stop, size_t nbins,
                          std::string const &label) {

  BinningPtr bin_info = std::make_shared<Binning>();
  bin_info->axis_labels.push_back(label);

  auto startl = logbase<base>(start);
  auto stopl = logbase<base>(stop);

  double lwidth = (stopl - startl) / double(nbins);

  std::vector<Binning::BinExtents> bins =
      edges_to_extents(log_spaced_edges<base>(start, stop, nbins));

  Binning::log_trace("[log{}_space({},{},{})] startl={} stopl={} lwidth={}",
                     base == 0 ? "" : std::to_string(base), start, stop, nbins,
                     startl, stopl, lwidth);
  Binning::log_trace("  {}", str_via_ss(bins));

  // be careful not to capture bin_info or you will create a circular reference
  // for the shared_ptr
  bin_info->binning_function =
      [=](std::vector<double> const &x) -> Binning::index_t {
    NUIS_LOGGER_TRACE(
        "Binning",
        "[log{}_space({},{},{}).binning_function] x.size() = {}, x[0] = {}",
        base == 0 ? "" : std::to_string(base), start, stop, nbins, x.size(),
        x.size() ? x[0] : 0xdeadbeef);

    if (x.size() < 1) {
      Binning::log_warn("[log{}_space({},{},{}).binning_function] was passed "
                        "an empty projection "
                        "vector. Returning npos. Compile with "
                        "CMAKE_BUILD_TYPE=Debug to make this an exception.",
                        base == 0 ? "" : std::to_string(base), start, stop,
                        nbins);
#ifndef NUIS_NDEBUG
      throw TooFewProjectionsForBinning();
#endif
      return Binning::npos;
    }

    if (!std::isnormal(x[0])) {
      Binning::log_warn(
          "[log{}_space({},{},{}).binning_function] was passed an "
          "abnornmal number = {}. Returning npos. Compile with "
          "CMAKE_BUILD_TYPE=Debug to make this an exception.",
          base == 0 ? "" : std::to_string(base), start, stop, nbins, x[0]);
#ifndef NUIS_NDEBUG
      throw UnbinnableNumber();
#endif
      return Binning::npos;
    }

    if (x[0] < 0) {
      Binning::log_info(
          "[log{}_space({},{},{}).binning_function] was passed an "
          "unloggable number = {}. Returning npos. Compile with "
          "CMAKE_BUILD_TYPE=Debug to make this an exception.",
          base == 0 ? "" : std::to_string(base), start, stop, nbins, x[0]);
#ifndef NUIS_NDEBUG
      throw UnbinnableNumber();
#endif
      return Binning::npos;
    }

    auto xl = logbase<base>(x[0]);

    if (xl >= stopl) {
      NUIS_LOGGER_TRACE("Binning",
                        "[log{}_space({},{},{}).binning_function] {} above "
                        "extent. Returning npos.",
                        base == 0 ? "" : std::to_string(base), start, stop,
                        nbins, x[0]);
      return Binning::npos;
    }

    if (xl < startl) {
      NUIS_LOGGER_TRACE("Binning",
                        "[log{}_space({},{},{}).binning_function] {} below "
                        "extent. Returning npos.",
                        base == 0 ? "" : std::to_string(base), start, stop,
                        nbins, x[0]);
      return Binning::npos;
    }

    Binning::index_t bin = std::floor((xl - startl) / lwidth);
    NUIS_LOGGER_TRACE("Binning",
                      "[log{}_space({},{},{}).binning_function] Found bin: {}, "
                      "v = {} is in ({} -- {})",
                      base == 0 ? "" : std::to_string(base), start, stop, nbins,
                      bin, x[0], bins[bin][0].low, bins[bin][0].high);
    return bin;
  };

  bin_info->bins = bins;
  return bin_info;
}

BinningPtr Binning::log10_space(double start, double stop, size_t nbins,
                                std::string const &label) {
  return log_space_impl<10>(start, stop, nbins, label);
}

BinningPtr Binning::ln_space(double start, double stop, size_t nbins,
                             std::string const &label) {
  return log_space_impl<0>(start, stop, nbins, label);
}

BinningPtr Binning::contiguous(std::vector<double> const &edges,
                               std::string const &label) {

  BinningPtr bin_info = std::make_shared<Binning>();

  auto bins = edges_to_extents(edges);

  bin_info->axis_labels.push_back(label);

  // be careful not to capture bin_info or you will create a circular reference
  // for the shared_ptr
  bin_info->binning_function = [=](std::vector<double> const &x) -> index_t {
    if (x.size() < 1) {
      log_warn("[contiguous.binning_function] was passed an empty projection "
               "vector. Returning npos. Compile with "
               "CMAKE_BUILD_TYPE=Debug to make this an exception.");
#ifndef NUIS_NDEBUG
      throw TooFewProjectionsForBinning();
#endif
      return npos;
    }

    if ((x[0] != 0) && !std::isnormal(x[0])) {
      log_warn("[contiguous.binning_function] was passed an "
               "abnornmal number = {}. Returning npos. Compile with "
               "CMAKE_BUILD_TYPE=Debug to make this an exception.",
               x[0]);
#ifndef NUIS_NDEBUG
      throw UnbinnableNumber();
#endif
      return npos;
    }

    if (x[0] < bins.front()[0].low) {
      NUIS_LOG_TRACE(
          "[contiguous.binning_function] x = {} < binning low edge = {}, "
          "returning npos.",
          x[0], bins.front()[0].low);
      return npos;
    }
    if (x[0] >= bins.back()[0].high) {
      NUIS_LOG_TRACE(
          "[contiguous.binning_function] x = {} >= binning high edge = {}, "
          "returning npos.",
          x[0], bins.back()[0].high);
      return npos;
    }

    // binary search for bin
    size_t L = 0;
    size_t R = bins.size() - 1;
    NUIS_LOG_TRACE(
        "[contiguous.binning_function]: begin binary search: {{ x: {}, L: "
        "{}, R: {} }}.",
        x[0], L, R);
    while (L <= R) {
      size_t m = std::floor((L + R) / 2);
      NUIS_LOG_TRACE(
          "[contiguous.binning_function]: checking {{ x: {}, m: {}, low: "
          "{}, high: {}}}.",
          x[0], m, bins[m][0].low, bins[m][0].high);
      if ((bins[m][0].low <= x[0]) && (bins[m][0].high > x[0])) {
        NUIS_LOG_TRACE(
            "[contiguous.binning_function]: binary search succeeded, in "
            "bin: {} < {} < {}",
            bins[m][0].low, bins[m][0].high, x[0]);
        return m;
      } else if (bins[m][0].high <= x[0]) {
        L = m + 1;
        NUIS_LOG_TRACE("  -- {} < {}: L = {}", bins[m][0].high, x[0], L);
      } else if (bins[m][0].low > x[0]) {
        R = m - 1;
        NUIS_LOG_TRACE("  -- {} > {}: R = {}", bins[m][0].low, x[0], R);
      }
    }
    NUIS_LOG_TRACE(
        "[contiguous.binning_function]: binary search failed, returning npos.");
    return npos;
  };

  bin_info->bins = bins;
  return bin_info;
}

struct from_extentsHelper : public nuis_named_log("Binning") {

  std::vector<Binning::BinExtents> bins;
  std::pair<size_t, std::vector<std::vector<Binning::index_t>>> bin_columns;

  size_t nax;
  size_t nbins;

  from_extentsHelper(std::vector<Binning::BinExtents> const &bi) : bins(bi) {

    if (!bins.size()) {
      log_critical("[from_extentsHelper]: Passed empty bins.");
      throw EmptyBinning();
    }

    nax = bins.front().size();
    nbins = bins.size();

    bin_columns = get_bin_columns(bins);
  }

  Binning::index_t operator()(std::vector<double> const &x) const {

    if (x.size() < nax) {
      log_critical("[from_extentsHelper]: projections passed in: {} is "
                   "smaller than the number of axes in a bin: {}",
                   x, nax);
      throw MismatchedAxisCount();
    }

    auto longax = bin_columns.first;
    bool found_column = false;

    NUIS_LOG_TRACE(">>>>>>>>>>>>>>>>>>>>> Search begin x = {}", x);

    for (size_t col_it = 0; col_it < bin_columns.second.size(); ++col_it) {
      auto const &col = bin_columns.second[col_it];
      auto const &ext = bins[col.front()][longax];

      if (ext.contains(x[longax])) {

#ifndef NUIS_NDEBUG
        NUIS_LOG_TRACE("[from_extentsHelper]: "
                       "++ x[{}] = {} in Column {}, {}",
                       longax, x[longax], col_it, str_via_ss(ext));
#endif

        found_column = true;

        for (Binning::index_t bi_it : col) {
          bool found_bin = true;
#ifndef NUIS_NDEBUG
          NUIS_LOG_TRACE("[from_extentsHelper]:   checking x = {} in bin ={}",
                         x, str_via_ss(bins[bi_it]));
#endif
          for (size_t ax_it = 0; ax_it < nax; ++ax_it) {
            if (!bins[bi_it][ax_it].contains(x[ax_it])) {
              found_bin = false;
              break;
            }
          }
          if (found_bin) {
            NUIS_LOG_TRACE("[from_extentsHelper]: Found bin x = {} in {}", x,
                           str_via_ss(bins[bi_it]));
            NUIS_LOG_TRACE("<<<<<<<<<<<<<<<<<<<<< Search end");
            return bi_it;
          }
#ifndef NUIS_NDEBUG
          else {
            NUIS_LOG_TRACE("[from_extentsHelper]:   -- Nope.");
          }
#endif
        }
      } else {
        NUIS_LOG_TRACE("[from_extentsHelper]: "
                       "-- x[{}] = {} not in Column {}, {}",
                       longax, x[longax], col_it, str_via_ss(ext));

        // we should be able to make this optimization but can't currently figure out how to order the bins correctly.
        (void)found_column;
        // if (found_column) { // left the columns that can contain this value
        //   break;
        // }
      }
    }

    NUIS_LOG_TRACE("[from_extentsHelper]: returning npos");
    NUIS_LOG_TRACE("<<<<<<<<<<<<<<<<<<<<< Search end");

    return Binning::npos;
  }
};

BinningPtr Binning::from_extents(std::vector<BinExtents> bins,
                                 std::vector<std::string> const &labels) {

  if (!bins.size()) {
    log_critical("from_extents passed an empty bin vector.");
    throw EmptyBinning();
  }

  BinningPtr bin_info = std::make_shared<Binning>();
  bin_info->axis_labels = labels;
  for (size_t i = bin_info->axis_labels.size(); i < bins.front().size(); ++i) {
    bin_info->axis_labels.push_back("");
  }

  auto const &sorted_unique_bins = unique(bins);

  if (bins.size() != sorted_unique_bins.size()) {
    log_critical("[from_extents]: When building Binning from vector of "
                 "BinExtents, the list of unique bins was {} long, while "
                 "the original list was {}. Binnings must be unique.",
                 sorted_unique_bins.size(), bins.size());
    log_critical("Bins: {}", str_via_ss(bins));
    throw BinningNotUnique();
  }

  if (binning_has_overlaps(bins)) {
    log_critical("[from_extents]: When building Binning from vector of "
                 "BinExtents, the list of bins appears to contain "
                 "overlaps. Binnings must be non-overlapping.");
    log_critical("Bins: {}", str_via_ss(bins));
    throw BinningHasOverlaps();
  }

  bin_info->binning_function = from_extentsHelper(bins);

  bin_info->bins = bins;
  return bin_info;
}

BinningPtr Binning::brute_force(std::vector<BinExtents> bins,
                                std::vector<std::string> const &labels) {

  if (!bins.size()) {
    log_critical("brute_force passed an empty bin vector.");
    throw EmptyBinning();
  }

  BinningPtr bin_info = std::make_shared<Binning>();
  bin_info->axis_labels = labels;
  for (size_t i = bin_info->axis_labels.size(); i < bins.front().size(); ++i) {
    bin_info->axis_labels.push_back("");
  }

  auto const &sorted_unique_bins = unique(bins);

  if (bins.size() != sorted_unique_bins.size()) {
    log_critical("[brute_force]: When building Binning from vector of "
                 "BinExtents, the list of unique bins was {} long, while "
                 "the original list was {}. Binnings must be unique.",
                 sorted_unique_bins.size(), bins.size());
    log_critical("Bins: {}", str_via_ss(bins));
    throw BinningNotUnique();
  }

  if (binning_has_overlaps(bins)) {
    log_critical("[brute_force]: When building Binning from vector of "
                 "BinExtents, the list of bins appears to contain "
                 "overlaps. Binnings must be non-overlapping.");
    log_critical("Bins: {}", str_via_ss(bins));
    throw BinningHasOverlaps();
  }

  bin_info->binning_function =
      [=](std::vector<double> const &x) -> Binning::index_t {
    for (size_t i = 0; i < bins.size(); i++) {
      auto const &bin_info_slice = bins[i];

      bool goodbin = true;
      for (size_t j = 0; j < bin_info_slice.size(); j++) {
        if (x[j] < bin_info_slice[j].low) {
          goodbin = false;
          break;
        }
        if (x[j] >= bin_info_slice[j].high) {
          goodbin = false;
          break;
        }
      }
      if (!goodbin) {
        continue;
      }
      return i;
    }
    return Binning::npos;
  };

  bin_info->bins = bins;
  return bin_info;
}

std::vector<Binning::BinExtents>
binning_product_recursive(std::vector<BinningPtr>::const_reverse_iterator from,
                          std::vector<BinningPtr>::const_reverse_iterator to) {

  if (from == to) {
    return {};
  }

  auto lower_extents = binning_product_recursive(from + 1, to);

  std::vector<Binning::BinExtents> bins;
  if (lower_extents.size()) {
    for (auto bin : (*from)->bins) {
      for (auto lbin : lower_extents) {
        bins.emplace_back();
        std::copy(lbin.begin(), lbin.end(), std::back_inserter(bins.back()));
        std::copy(bin.begin(), bin.end(), std::back_inserter(bins.back()));
      }
    }
  } else {
    return (*from)->bins;
  }

  return bins;
}

BinningPtr Binning::product(std::vector<BinningPtr> binnings) {

  BinningPtr bin_info_product = std::make_shared<Binning>();

  size_t nax = 0;
  size_t nbins = 1;
  std::vector<size_t> nbins_in_binning_slice = {1};
  std::vector<size_t> nax_in_binning = {};

  for (auto const &bin_info : binnings) {
    auto nax_in_this_binning = bin_info->bins.front().size();
    nax += nax_in_this_binning;
    nax_in_binning.push_back(nax_in_this_binning);

    auto nbins_in_binning = bin_info->bins.size();
    nbins *= nbins_in_binning;
    nbins_in_binning_slice.push_back(nbins);

    // combining all bin_info labels
    std::copy(bin_info->axis_labels.begin(), bin_info->axis_labels.end(),
              std::back_inserter(bin_info_product->axis_labels));
  }

  bin_info_product->binning_function =
      [=](std::vector<double> const &x) -> index_t {
    if (x.size() < nax) {
      log_critical("[product]: projections passed in: {} is "
                   "smaller than the number of axes in a bin: {}",
                   x, nax);
      throw MismatchedAxisCount();
    }

    index_t gbin = 0;
    size_t nax_consumed = 0;
    for (size_t binning_it = 0; binning_it < binnings.size(); ++binning_it) {

      index_t binning_bin =
          binnings[binning_it]->binning_function(std::vector<double>(
              x.begin() + nax_consumed,
              x.begin() + nax_consumed + nax_in_binning[binning_it]));

      if (binning_bin == npos) {
        NUIS_LOG_TRACE(
            "[product]: sub-binning[{}] returned npos. Returning npos",
            binning_it);
        return npos;
      }

      gbin += binning_bin * nbins_in_binning_slice[binning_it];
      nax_consumed += nax_in_binning[binning_it];

      NUIS_LOG_TRACE(
          "[product]: sub-binning[{}] found bin: {} after consuming {} axes of "
          "projections. \n\tTotal of {}/{} consumed.\n\tRunning gbin = {}",
          binning_it, binning_bin, nax_in_binning[binning_it], nax_consumed,
          x.size(), gbin);
    }

    NUIS_LOG_TRACE("[product]: Returning gbin {}", gbin);
    return gbin;
  };
  bin_info_product->bins =
      binning_product_recursive(binnings.rbegin(), binnings.rend());
  return bin_info_product;
}
} // namespace nuis