#include "nuis/histframe/Binning.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

#include <cmath>

namespace nuis {
namespace Bins {

Eigen::ArrayXd BinningInfo::bin_sizes() const {
  Eigen::ArrayXd bin_sizes = Eigen::ArrayXd::Zero(extents.size());
  size_t i = 0;
  for (auto const &bin : extents) {
    bin_sizes[i] = 1;
    for (auto const &ext : bin) {
      bin_sizes[i] *= ext.width();
    }
    i++;
  }
  return bin_sizes;
}

std::vector<std::vector<BinningInfo::extent>> build_extents(
    // std::vector<BinOp>::const_reverse_iterator start,
    //             std::vector<BinOp>::const_reverse_iterator end,
    std::vector<BinOp>::const_reverse_iterator from,
    std::vector<BinOp>::const_reverse_iterator to) {

  std::vector<std::vector<BinningInfo::extent>> extents;

  // spdlog::info("build_extents({},{}):", from - start, to - start);
  if (from == to) {
    // spdlog::info("  -- reached the end");
    return {};
  }

  auto lower_extents = build_extents(
      // start, end,
      from + 1, to);

  // size_t i = 0;
  if (lower_extents.size()) {
    for (auto bin : from->bin_info.extents) {
      // spdlog::info("for outer bin {} on ax {}:", i++, end - to);

      for (auto lbin : lower_extents) {
        extents.emplace_back();
        std::copy(lbin.begin(), lbin.end(), std::back_inserter(extents.back()));
        std::copy(bin.begin(), bin.end(), std::back_inserter(extents.back()));
        // std::stringstream ss;
        // ss << " adding axis bins: [ ";
        // for (auto bi : extents.back()) {
        //   ss << bi << ", ";
        // }
        // ss << "]";
        // spdlog::info(ss.str());
      }
    }
  } else {
    return from->bin_info.extents;
  }
  return extents;
}

BinOp combine(std::vector<BinOp> const &ops) {

  BinningInfo bin_info;

  size_t nax = 0;
  size_t nbins = 1;
  std::vector<size_t> nbins_in_op_slice = {1};
  std::vector<size_t> nax_in_op = {};
  for (auto const &op : ops) {
    auto ndims_in_op = op.bin_info.extents.front().size();
    nax += ndims_in_op;
    nax_in_op.push_back(ndims_in_op);
    auto nbins_in_op = op.bin_info.extents.size();
    nbins *= nbins_in_op; // nbins in op
    nbins_in_op_slice.push_back(nbins);

    // combining all op labels
    std::copy(op.bin_info.axis_labels.begin(), op.bin_info.axis_labels.end(),
              std::back_inserter(bin_info.axis_labels));
  }
  bin_info.extents = build_extents(
      // ops.rbegin(), ops.rend(),
      ops.rbegin(), ops.rend());

  return {bin_info, [=](std::vector<double> const &x) -> BinId {
            if (x.size() < nax) {
              return npos;
            }

            // spdlog::info("Finding gbin for {}", x);

            BinId gbin = 0;
            size_t nax_consumed = 0;
            for (size_t op_it = 0; op_it < nax_in_op.size(); ++op_it) {

              // spdlog::info("  op[{}], nax_in_op = {}, being passed: {}",
              // op_it,
              //              nax_in_op[op_it],
              //              std::vector<double>(x.begin() + nax_consumed,
              //                                  x.begin() + nax_consumed +
              //                                      nax_in_op[op_it]));

              BinId op_bin = ops[op_it].bin_func(std::vector<double>(
                  x.begin() + nax_consumed,
                  x.begin() + nax_consumed + nax_in_op[op_it]));

              gbin += op_bin * nbins_in_op_slice[op_it];
              nax_consumed += nax_in_op[op_it];

              if (op_bin == npos) {
                // spdlog::info("  OOR on dim: {}.", ax_i);
                return npos;
              }
            }

            // spdlog::info("gbin = {}", gbin);

            return gbin;
          }};
}

BinOp lin_space(size_t nbins, double min, double max,
                std::string const &label) {
  double step = (max - min) / double(nbins);

  BinningInfo bin_info;
  bin_info.axis_labels.push_back(label);

  for (size_t i = 0; i < nbins; ++i) {
    bin_info.extents.emplace_back();
    bin_info.extents.back().emplace_back(
        BinningInfo::extent{min + (i * step), min + ((i + 1) * step)});
  }

  return {bin_info, [=](std::vector<double> const &x) -> BinId {
            if (x.size() < 1) {
              return npos;
            }

            return (x[0] >= max)  ? npos
                   : (x[0] < min) ? npos
                                  : std::floor((x[0] - min) / step);
          }};
}

BinOp lin_spaceND(std::vector<std::tuple<size_t, double, double>> axes,
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

  BinningInfo bin_info;
  for (size_t i = 0; i < nax; ++i) {
    bin_info.axis_labels.push_back(labels.size() > i ? labels[i] : "");
  }

  std::vector<BinId> dimbins(nax, 0);

  for (size_t bin_i = 0; bin_i < nbins; ++bin_i) {
    bin_info.extents.emplace_back(nax, BinningInfo::extent{});

    size_t bin_remainder = bin_i;
    for (int ax_i = int(nax - 1); ax_i >= 0; --ax_i) {

      dimbins[ax_i] = (bin_remainder / nbins_in_slice[ax_i]);
      bin_remainder = bin_remainder % nbins_in_slice[ax_i];

      bin_info.extents.back()[ax_i] = BinningInfo::extent{
          std::get<1>(axes[ax_i]) + (dimbins[ax_i] * steps[ax_i]),
          std::get<1>(axes[ax_i]) + ((dimbins[ax_i] + 1) * steps[ax_i])};
    }
  }

  return {bin_info, [=](std::vector<double> const &x) -> BinId {
            if (x.size() < nax) {
              return npos;
            }

            // spdlog::info("Finding gbin for {}", x);

            BinId gbin = 0;
            for (size_t ax_i = 0; ax_i < nax; ++ax_i) {

              BinId dimbin =
                  (x[ax_i] >= std::get<2>(axes[ax_i])) ? npos
                  : (x[ax_i] < std::get<1>(axes[ax_i]))
                      ? npos
                      : std::floor((x[ax_i] - std::get<1>(axes[ax_i])) /
                                   steps[ax_i]);
              gbin += dimbin * nbins_in_slice[ax_i];

              // spdlog::info("  x[{0}] = {1}, dimbin[{0}] = {2}({5}--{6}), "
              //              "extra_gbin = {3} ({4})",
              //              ax_i, x[ax_i], dimbin, dimbin *
              //              nbins_in_slice[ax_i], nbins_in_slice[ax_i],
              //              std::get<1>(axes[ax_i]) + (dimbin * steps[ax_i]),
              //              std::get<1>(axes[ax_i]) + ((dimbin + 1) *
              //              steps[ax_i]));

              if (dimbin == npos) {
                // spdlog::info("  OOR on dim: {}.", ax_i);
                return npos;
              }
            }

            // spdlog::info("gbin = {}", gbin);

            return gbin;
          }};
}

template <int base>
BinOp log_space_impl(size_t nbins, double min, double max,
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

  BinningInfo bin_info;
  bin_info.axis_labels.push_back(label);

  for (size_t i = 0; i < nbins; ++i) {
    bin_info.extents.emplace_back();

    auto low = expbase(minl + (i * step));
    auto high = expbase(minl + ((i + 1) * step));
    // spdlog::info(
    //     "[log_space<base = {}>]:\n\tparams: {{ nbins: {}, min: {:.2f}, max: "
    //     "{:.2f}, minl: {:.2f}, maxl: {:.2f}, step: {:.2f} }}. bin {} = [ "
    //     "{:.2f} "
    //     "-- {:.2f} ] "
    //     "| [ {:.2f} -- {:.2f} ].",
    //     base, nbins, min, max, minl, maxl, step, i, low, high,
    //     minl + (i * step), maxl + ((i + 1) * step));

    bin_info.extents.back().emplace_back(BinningInfo::extent{low, high});
  }

  return {bin_info, [=](std::vector<double> const &x) -> BinId {
            if (x.size() < 1) {
              return npos;
            }

            // spdlog::info("[log_space<base = {}>]: Finding bin for {}", base,
            // x);

            if (x[0] <= 0) {
              spdlog::warn("[log_space<base = {}>]: Attempted to find log bin "
                           "for unloggable number, {} ",
                           (base == 0 ? "e" : std::to_string(base)), x[0]);
              return npos;
            }

            auto xl = logbase(x[0]);

            return (xl >= maxl)  ? npos
                   : (xl < minl) ? npos
                                 : std::floor((xl - minl) / step);
          }};
}

BinOp log10_space(size_t nbins, double min, double max,
                  std::string const &label) {
  return log_space_impl<10>(nbins, min, max, label);
}

BinOp log_space(size_t nbins, double min, double max,
                std::string const &label) {
  return log_space_impl<0>(nbins, min, max, label);
}

BinOp from_extents1D(std::vector<BinningInfo::extent> extents,
                     std::string const &label) {

  // PS. Unsure if this is the right way to do this.
  // Users could make a list and assume bin1 is their original order.
  // We should assume the order passed in even if the search is inefficient.
  std::sort(extents.begin(), extents.end()); 

  BinningInfo bin_info;
  bin_info.axis_labels.push_back(label);
  for (auto ext : extents) {

    bin_info.extents.emplace_back();
    bin_info.extents.back().emplace_back(ext);
  }

  return {bin_info, [=](std::vector<double> const &x) -> BinId {

            size_t L = 0;
            size_t R = bin_info.extents.size() - 1;
            // spdlog::info(
            // "[from_extents]: binary search: {{ x: {}, L: {}, R: {} }}.",
            // x[0], L, R);
            while (L <= R) {
              size_t m = floor((L + R) / 2);
              // spdlog::info(
              //     "[from_extents]: checking {{ x: {}, m: {}, min: {}, max: {}
              //     }}.",
              // x[0], m, bin_info.extents[m][0].min,
              // bin_info.extents[m][0].max);
              if ((bin_info.extents[m][0].min <= x[0]) &&
                  (bin_info.extents[m][0].max > x[0])) {
                // spdlog::info("-- in bin: {} < {} < {}",
                // bin_info.extents[m][0].min,
                //              bin_info.extents[m][0].max, x[0]);
                return m;
              } else if (bin_info.extents[m][0].max <= x[0]) {
                L = m + 1;
                // spdlog::info("-- {} < {}: L = {}",
                // bin_info.extents[m][0].max, x[0], L);
              } else if (bin_info.extents[m][0].min > x[0]) {
                R = m - 1;
                // spdlog::info("-- {} > {}: R = {}",
                // bin_info.extents[m][0].min, x[0],
                //              R);
              }
            }
            // spdlog::info("[from_extents]: binary search: failed, returning
            // npos.");
            return npos;
          }};
}

BinOp from_binedges1D(std::vector<double> const &edges, std::string const &label) {
  std::vector<BinningInfo::extent> extents;
  for (size_t i = 1; i < edges.size(); ++i) {
    extents.push_back(BinningInfo::extent{edges[i - 1], edges[i]});
  }
  return from_extents1D(extents, label);
}


} // namespace Bins
} // namespace nuis

std::ostream &operator<<(std::ostream &os,
                         nuis::Bins::BinningInfo::extent const &ext) {
  return os << fmt::format("({:.2f} - {:.2f})", ext.min, ext.max);
}

std::ostream &operator<<(std::ostream &os, nuis::Bins::BinningInfo const &bi) {
  os << fmt::format("Axis lables: {}\nBins: [\n", bi.axis_labels);
  for (size_t i = 0; i < bi.extents.size(); ++i) {
    os << fmt::format("  {}: [", i);
    for (size_t j = 0; j < bi.extents[i].size(); ++j) {
      os << bi.extents[i][j]
         << ((j + 1) == bi.extents[i].size() ? "]\n" : ", ");
    }
  }
  return os << "]" << std::endl;
}