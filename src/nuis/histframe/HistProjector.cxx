#include "nuis/histframe/HistProjector.h"

#include "fmt/ranges.h"

#include "spdlog/spdlog.h"

namespace nuis {
namespace Bins {

struct ProjectionMap {
  std::vector<size_t> project_to_axes;
  std::map<BinningInfo::extent, std::vector<BinId>> bin_columns;
};

ProjectionMap Build1DProjectionMap(BinningInfo const &bin_info,
                                   size_t proj_to_axis) {
  ProjectionMap pmap;
  pmap.project_to_axes.push_back(proj_to_axis);

  for (BinId bi_it = 0; bi_it < BinId(bin_info.extents.size()); ++bi_it) {

    auto const &bin = bin_info.extents[bi_it];

    // get the bin extent along the projection axis
    for (size_t ax_it = 0; ax_it < bin.size(); ++ax_it) {
      if (ax_it == proj_to_axis) {
        pmap.bin_columns[bin[ax_it]].push_back(bi_it);
      }
    }
  }

  // do heuristics checks

  // check nbins
  size_t nbins = 0;
  size_t i = 0;
  for (auto const &[proj_extent, bins] : pmap.bin_columns) {
    if (!i++) {
      nbins = bins.size();
    } else {
      if (nbins != bins.size()) {
        spdlog::warn(
            "[Build1DProjectionMap]: Projection axis new bin {} has {} bins in "
            "its projection column, but the first new bin had {}. This "
            "suggests that this projection map might not be not correct.",
            i, bins.size(), nbins);
      }
    }
  }

  // check stride
  size_t stride = 0;
  i = 0;
  for (auto const &[proj_extent, bins] : pmap.bin_columns) {
    if (bins.size() < 2) {
      continue;
    }
    if (!stride) {
      stride = bins[1] - bins[0];
    }
    for (size_t bi_it = 1; bi_it < bins.size(); ++bi_it) {
      if (stride != (bins[bi_it] - bins[bi_it - 1])) {
        spdlog::warn(
            "[Build1DProjectionMap]: Projection axis new bin {}, projection "
            "column bin {} to {} has stride {}, but the first stride in the "
            "first column was {}. This "
            "suggests that this projection map might not be not correct.",
            i, bi_it - 1, bi_it, (bins[bi_it] - bins[bi_it - 1]), stride);
      }
    }
    i++;
  }

  return pmap;
}

std::vector<Bins::BinningInfo::extent>
GetAxisExtents(BinningInfo const &bin_info, size_t proj_to_axis) {
  std::vector<Bins::BinningInfo::extent> bin_extents;

  for (auto const &bin : bin_info.extents) {
    if (bin.size() <= proj_to_axis) {
      spdlog::critical("Tried to get dimension {} extent from binning with "
                       "only {} dimensions.",
                       proj_to_axis, bin.size());
      abort();
    }
    bin_extents.push_back(bin[proj_to_axis]);
  }

  std::stable_sort(bin_extents.begin(), bin_extents.end());
  bin_extents.erase(std::unique(bin_extents.begin(), bin_extents.end()),
                    bin_extents.end());
  return bin_extents;
}

} // namespace Bins
} // namespace nuis

std::ostream &operator<<(std::ostream &os,
                         nuis::Bins::ProjectionMap const &pm) {
  os << "{ Project onto axis: " << pm.project_to_axes.front() << "\n";
  int i = 0;
  for (auto const &[proj_extent, bins] : pm.bin_columns) {
    os << "  { projected bin: " << (i++) << ", extent: " << proj_extent
       << fmt::format(", original_bins: {} }}\n", bins);
  }
  return os << "}" << std::endl;
}

namespace nuis {

HistFrame Project1D(HistFrame const &hf, size_t proj_to_axis) {
  auto const &pm = Build1DProjectionMap(hf.binning.bin_info, proj_to_axis);
  auto const &extents = GetAxisExtents(hf.binning.bin_info, proj_to_axis);
  HistFrame projhf(Bins::from_extents1D(
      extents, hf.binning.bin_info.axis_labels[proj_to_axis]));

  if (pm.bin_columns.size() != extents.size()) {
    spdlog::critical("[Project1D]: Number of bins in the projection map, {}, "
                     "is not equal to the number of bins in the axis extents, "
                     "{}. Is this definitely the right projection map?");
    std::cout << "BinInfo: " << projhf.binning.bin_info << std::endl;
    std::cout << "Projection map: " << pm << std::endl;
    abort();
  }

  projhf.column_info = hf.column_info;
  projhf.reset();

  for (size_t row_i = 0; row_i < extents.size(); ++row_i) {
    if (!pm.bin_columns.count(extents[row_i])) {
      spdlog::critical(
          "[Project1D]; Something has got out of whack here. The two methods "
          "of finding the projected binning have disagreed. This is a bug.");
      abort();
    }
    for (auto const &bi_it : pm.bin_columns.at(extents[row_i])) {
      for (int col_i = 0; col_i < hf.contents.cols(); ++col_i) {
        projhf.contents(row_i, col_i) += hf.contents(bi_it, col_i);
        projhf.variance(row_i, col_i) += hf.variance(bi_it, col_i);
      }
    }
  }

  projhf.nfills = hf.nfills;

  return projhf;
}

} // namespace nuis
