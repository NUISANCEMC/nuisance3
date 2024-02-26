#include "nuis/histframe/Binning.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

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
            if (x.size() != 1) {
              return npos;
            }
            return (x[0] > max)   ? npos
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
    bin_info.extents.emplace_back();

    size_t bin_remainder = bin_i;
    for (int ax_i = int(nax - 1); ax_i >= 0; --ax_i) {

      dimbins[ax_i] = (bin_remainder / nbins_in_slice[ax_i]);
      bin_remainder = bin_remainder % nbins_in_slice[ax_i];

      bin_info.extents.back().emplace_back(BinningInfo::extent{
          std::get<1>(axes[ax_i]) + (dimbins[ax_i] * steps[ax_i]),
          std::get<1>(axes[ax_i]) + ((dimbins[ax_i] + 1) * steps[ax_i])});
    }
  }

  return {bin_info, [=](std::vector<double> const &x) -> BinId {
            if (x.size() != nax) {
              return npos;
            }

            // spdlog::info("Finding gbin for {}", x);

            BinId gbin = 0;
            for (size_t ax_i = 0; ax_i < nax; ++ax_i) {

              BinId dimbin =
                  (x[ax_i] > std::get<2>(axes[ax_i])) ? npos
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

} // namespace Bins
} // namespace nuis