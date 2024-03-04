#include "nuis/histframe/Binning.h"
#include "nuis/histframe/BinningUtility.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

#include <cmath>

namespace nuis {

std::vector<Binning::BinExtents>
binning_product_recursive(std::vector<Binning>::const_reverse_iterator from,
                          std::vector<Binning>::const_reverse_iterator to) {

  if (from == to) {
    return {};
  }

  auto lower_extents = binning_product_recursive(from + 1, to);

  std::vector<Binning::BinExtents> bins;
  if (lower_extents.size()) {
    for (auto bin : from->bins) {
      for (auto lbin : lower_extents) {
        bins.emplace_back();
        std::copy(lbin.begin(), lbin.end(), std::back_inserter(bins.back()));
        std::copy(bin.begin(), bin.end(), std::back_inserter(bins.back()));
      }
    }
  } else {
    return from->bins;
  }

  return bins;
}

Eigen::ArrayXd Binning::bin_sizes() const {
  Eigen::ArrayXd bin_sizes = Eigen::ArrayXd::Zero(bins.size());
  size_t i = 0;
  for (auto const &bin : bins) {
    bin_sizes[i] = 1;
    for (auto const &ext : bin) {
      bin_sizes[i] *= ext.width();
    }
    i++;
  }
  return bin_sizes;
}

Binning Binning::lin_space(size_t nbins, double min, double max,
                           std::string const &label) {
  double step = (max - min) / double(nbins);

  Binning bin_info;
  bin_info.axis_labels.push_back(label);

  for (size_t i = 0; i < nbins; ++i) {
    bin_info.bins.emplace_back();
    bin_info.bins.back().emplace_back(
        SingleExtent{min + (i * step), min + ((i + 1) * step)});
  }

  bin_info.func = [=](std::vector<double> const &x) -> Index {
    if (x.size() < 1) {
      return npos;
    }

    return (x[0] >= max)  ? npos
           : (x[0] < min) ? npos
                          : std::floor((x[0] - min) / step);
  };
  return bin_info;
}

Binning
Binning::lin_spaceND(std::vector<std::tuple<size_t, double, double>> axes,
                     std::vector<std::string> labels) {

  size_t nax = axes.size();
  std::vector<size_t> nbins_in_slice = {1};
  std::vector<size_t> steps;

  size_t nbins = 1;
  for (size_t ax_i = 0; ax_i < nax; ++ax_i) {
    steps.push_back((std::get<2>(axes[ax_i]) - std::get<1>(axes[ax_i])) /
                    double(std::get<0>(axes[ax_i])));
    nbins *= std::get<0>(axes[ax_i]);
    nbins_in_slice.push_back(nbins);
  }

  Binning bin_info;
  for (size_t i = 0; i < nax; ++i) {
    bin_info.axis_labels.push_back(labels.size() > i ? labels[i] : "");
  }

  std::vector<Index> dimbins(nax, 0);

  for (size_t bin_i = 0; bin_i < nbins; ++bin_i) {
    bin_info.bins.emplace_back(nax, SingleExtent{});

    size_t bin_remainder = bin_i;
    for (int ax_i = int(nax - 1); ax_i >= 0; --ax_i) {

      dimbins[ax_i] = (bin_remainder / nbins_in_slice[ax_i]);
      bin_remainder = bin_remainder % nbins_in_slice[ax_i];

      bin_info.bins.back()[ax_i] = SingleExtent{
          std::get<1>(axes[ax_i]) + (dimbins[ax_i] * steps[ax_i]),
          std::get<1>(axes[ax_i]) + ((dimbins[ax_i] + 1) * steps[ax_i])};
    }
  }

  bin_info.func = [=](std::vector<double> const &x) -> Index {
    if (x.size() < nax) {
      return npos;
    }

    Index gbin = 0;
    for (size_t ax_i = 0; ax_i < nax; ++ax_i) {

      Index dimbin =
          (x[ax_i] >= std::get<2>(axes[ax_i])) ? npos
          : (x[ax_i] < std::get<1>(axes[ax_i]))
              ? npos
              : std::floor((x[ax_i] - std::get<1>(axes[ax_i])) / steps[ax_i]);
      gbin += dimbin * nbins_in_slice[ax_i];

      if (dimbin == npos) {
        return npos;
      }
    }

    return gbin;
  };
  return bin_info;
}

template <int base>
Binning log_space_impl(size_t nbins, double min, double max,
                       std::string const &label) {

  auto logbase = [=](double v) -> double {
    return (base == 0) ? std::log(v) : (std::log(v) / std::log(base));
  };
  auto expbase = [=](double v) -> double {
    return (base == 0) ? std::exp(v) : std::exp(v * std::log(base));
  };

  auto minl = logbase(min);
  auto maxl = logbase(max);

  double step = (maxl - minl) / double(nbins);

  Binning bin_info;
  bin_info.axis_labels.push_back(label);

  for (size_t i = 0; i < nbins; ++i) {
    bin_info.bins.emplace_back();

    auto low = expbase(minl + (i * step));
    auto high = expbase(minl + ((i + 1) * step));

    bin_info.bins.back().emplace_back(Binning::SingleExtent{low, high});
  }

  bin_info.func = [=](std::vector<double> const &x) -> Binning::Index {
    if (x.size() < 1) {
      return Binning::npos;
    }

    if (x[0] <= 0) {
      spdlog::warn("[log_space<base = {}>]: Attempted to find log bin "
                   "for unloggable number, {} ",
                   (base == 0 ? "e" : std::to_string(base)), x[0]);
      return Binning::npos;
    }

    auto xl = logbase(x[0]);

    return (xl >= maxl)  ? Binning::npos
           : (xl < minl) ? Binning::npos
                         : std::floor((xl - minl) / step);
  };
  return bin_info;
}

Binning Binning::log10_space(size_t nbins, double min, double max,
                             std::string const &label) {
  return log_space_impl<10>(nbins, min, max, label);
}

Binning Binning::log_space(size_t nbins, double min, double max,
                           std::string const &label) {
  return log_space_impl<0>(nbins, min, max, label);
}

Binning Binning::contiguous(std::vector<double> const &edges,
                            std::string const &label) {

  Binning bin_info;

  for (size_t i = 1; i < edges.size(); ++i) {
    if (edges[i] <= edges[i - 1]) {
      spdlog::critical(
          "[contiguous]: Bin edges are not unique and monotonically "
          "increasing. edge[{}] = {}, edge[{}] = {}.",
          i, edges[i], i - 1, edges[i - 1]);
      abort();
    }
    bin_info.bins.emplace_back();
    bin_info.bins.back().push_back(SingleExtent{edges[i - 1], edges[i]});
  }

  bin_info.axis_labels.push_back(label);

  // binary search for bin
  bin_info.func = [=](std::vector<double> const &x) -> Index {
    size_t L = 0;
    size_t R = bin_info.bins.size() - 1;
    while (L <= R) {
      size_t m = floor((L + R) / 2);
      if (bin_info.bins[m][0].contains(x[0])) {
        return m;
      } else if (bin_info.bins[m][0].max <= x[0]) {
        L = m + 1;
      } else if (bin_info.bins[m][0].min > x[0]) {
        R = m - 1;
      }
    }
    return npos;
  };
  return bin_info;
}

Binning Binning::from_extents(std::vector<BinExtents> bins,
                              std::vector<std::string> const &labels) {

  Binning bin_info;
  bin_info.axis_labels = labels;
  for (size_t i = bin_info.axis_labels.size(); i < bins.front().size(); ++i) {
    bin_info.axis_labels.push_back("");
  }
  bin_info.bins = bins;

  auto const &sorted_unique_bins = unique(bins);

  if (bin_info.bins.size() != sorted_unique_bins.size()) {
    spdlog::critical("[from_extents]: When building Binning from vector of "
                     "BinExtents, the list of unique bins was {} long, while "
                     "the original list was {}. Binnings must be unique.");
    std::stringstream ss("");
    ss << bins;
    spdlog::critical("Bins: {}", ss.str());
    abort();
  }

  if (binning_has_overlaps(bin_info.bins)) {
    spdlog::critical("[from_extents]: When building Binning from vector of "
                     "BinExtents, the list of bins appears to contain "
                     "overlaps. Binnings must be non-overlapping.");
    std::stringstream ss("");
    ss << bins;
    spdlog::critical("Bins: {}", ss.str());
    abort();
  }

  std::vector<std::pair<Index, BinExtents>> sorted_bins;
  for (Index bi_it = 0; bi_it < Index(bins.size()); ++bi_it) {
    sorted_bins.emplace_back(bi_it, bins[bi_it]);
  }
  std::stable_sort(sorted_bins.begin(), sorted_bins.end(),
                   [](std::pair<Index, BinExtents> const &a,
                      std::pair<Index, BinExtents> const &b) {
                     auto const &a_exts = a.second;
                     auto const &b_exts = b.second;
                     if (a_exts.size() != b_exts.size()) {
                       spdlog::critical(
                           "[from_extents/stable_sort]: Tried to sort "
                           "multi-dimensional binning with "
                           "bins of unequal dimensionality: {} != {}",
                           a_exts.size(), b_exts.size());
                       abort();
                     }
                     for (size_t i = a_exts.size(); i > 0; i--) {
                       if (!(a_exts[i - 1] == b_exts[i - 1])) {
                         return a_exts[i - 1] < b_exts[i - 1];
                       }
                     }
                     // if we've got here the bin is identical in all dimensions
                     return false;
                   });

  std::function<std::pair<Index, Index>(std::vector<double> const &,
                                            Index, Index, size_t)>
      get_axis_bin_range = [sorted_bins, &get_axis_bin_range](
                               std::vector<double> const &x, Index from,
                               Index to, size_t ax) {
        spdlog::info(
            "[get_axis_bin_range]: x = {}, from = {}, to = {}, ax = {}", x[ax],
            from, to, ax);

        if (sorted_bins[from].second[ax].min >
            x[ax]) { // below any bins in this slice
          spdlog::info("[get_axis_bin_range]: x = {} < bins[from][ax].min = {}",
                       x[ax], sorted_bins[from].second[ax].min);
          return std::pair<Index, Index>{npos, npos};
        }

        auto stop = std::min(Index(sorted_bins.size()), to);

        if (sorted_bins[stop - 1].second[ax].max <
            x[ax]) { // above any bins in this slice
          spdlog::info(
              "[get_axis_bin_range]: x = {} < bins[stop - 1][ax].max = {}",
              x[ax], sorted_bins[stop - 1].second[ax].max);
          return std::pair<Index, Index>{npos, npos};
        }

        Index from_this_ax = npos;
        Index to_this_ax = npos;
        for (Index bi_it = from; bi_it < stop; ++bi_it) {
          if (sorted_bins[bi_it].second[ax].contains(x[ax])) {
            if (from_this_ax == npos) {
              spdlog::info("[get_axis_bin_range]: first bin in ax: {}", bi_it);
              from_this_ax = bi_it;
            }
            to_this_ax = bi_it;

          } else if (to_this_ax != npos) {
            break;
          }
        }
        spdlog::info("[get_axis_bin_range]: last bin in ax: {}", to_this_ax);
        return (ax == 0)
                   ? std::pair<Index, Index>{from_this_ax, to_this_ax}
                   : get_axis_bin_range(x, from_this_ax, to_this_ax, ax - 1);
      };

  bin_info.func = [sorted_bins, bins, &get_axis_bin_range](
                      std::vector<double> const &x) -> Index {
    auto contains_range = get_axis_bin_range(x, 0, sorted_bins.size(), 0);
    if ((contains_range.first == npos) || (contains_range.second == npos)) {
      return npos;
    }
    if (contains_range.first != contains_range.second) {
      spdlog::critical("[from_extents]: When searching for bin, failed to "
                       "find a unique bin. Either this is a bug in "
                       "NUISANCE or the binning is not unique.");
      std::stringstream ss;
      ss << "REPORT INFO:\n>>>----------------------------\ninput "
            "bins: \n"
         << bins << "\n";
      spdlog::critical(ss.str());
      spdlog::critical("searching for x: {}", x);
      spdlog::critical("get_axis_bin_range: {}", contains_range);
      abort();
    }
    // return the original Index
    return sorted_bins[contains_range.first].first;
  };
  return bin_info;
}

Binning Binning::product(std::vector<Binning> const &binnings) {

  Binning bin_info_product;

  size_t nax = 0;
  size_t nbins = 1;
  std::vector<size_t> nbins_in_op_slice = {1};
  std::vector<size_t> nax_in_op = {};

  for (auto const &bin_info : binnings) {
    auto ndims_in_op = bin_info.bins.front().size();
    nax += ndims_in_op;
    nax_in_op.push_back(ndims_in_op);

    auto nbins_in_op = bin_info.bins.size();
    nbins *= nbins_in_op;
    nbins_in_op_slice.push_back(nbins);

    // combining all bin_info labels
    std::copy(bin_info.axis_labels.begin(), bin_info.axis_labels.end(),
              std::back_inserter(bin_info_product.axis_labels));
  }
  bin_info_product.bins =
      binning_product_recursive(binnings.rbegin(), binnings.rend());

  bin_info_product.func = [=](std::vector<double> const &x) -> Index {
    if (x.size() < nax) {
      return npos;
    }

    Index gbin = 0;
    size_t nax_consumed = 0;
    for (size_t op_it = 0; op_it < nax_in_op.size(); ++op_it) {

      Index op_bin = binnings[op_it].func(
          std::vector<double>(x.begin() + nax_consumed,
                              x.begin() + nax_consumed + nax_in_op[op_it]));

      gbin += op_bin * nbins_in_op_slice[op_it];
      nax_consumed += nax_in_op[op_it];

      if (op_bin == npos) {
        return npos;
      }
    }

    return gbin;
  };
  return bin_info_product;
}

} // namespace nuis