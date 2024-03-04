#include "nuis/histframe/HistProjector.h"
#include "nuis/histframe/BinningUtility.h"

#include "fmt/ranges.h"

#include "spdlog/spdlog.h"

namespace nuis {

struct ProjectionMap {
  std::vector<size_t> project_to_axes;
  std::vector<Binning::BinExtents> projected_extents;
  std::map<Binning::BinExtents, std::vector<Binning::Index>> bin_columns;
};

ProjectionMap BuildProjectionMap(Binning const &bin_info,
                                 std::vector<size_t> proj_to_axes) {
  ProjectionMap pmap;
  pmap.project_to_axes = proj_to_axes;
  pmap.projected_extents = project_to_unique_bins(bin_info.bins, proj_to_axes);

  for (auto &proj_bin : pmap.projected_extents) {
    pmap.bin_columns[proj_bin] = std::vector<Binning::Index>{};
  }

  for (Binning::Index bi_it = 0;
       bi_it < Binning::Index(bin_info.bins.size()); ++bi_it) {

    auto const &bin = bin_info.bins[bi_it];

    Binning::BinExtents proj_bin;
    for (auto proj_to_axis : proj_to_axes) {
      proj_bin.push_back(bin[proj_to_axis]);
    }

    if (!pmap.bin_columns.count(proj_bin)) {
      spdlog::critical(
          "[BuildProjectionMap]: When scanning bins, built projected bin "
          "extent that project_to_unique_bins did not find, this is a bug in "
          "NUISANCE, please report it to the authors.");
      std::stringstream ss;
      ss << "REPORT INFO:\n>>>----------------------------\ninput bin_info:\n"
         << bin_info << "\n";
      spdlog::critical(ss.str());
      ss.str("");
      ss << "projected extents: " << pmap.projected_extents << "\n";
      spdlog::critical(ss.str());
      ss.str("");
      ss << "missed bin: " << proj_bin << "\n----------------------------<<<\n";
      abort();
    }
    pmap.bin_columns.at(proj_bin).push_back(bi_it);
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

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::ProjectionMap const &pm) {
  os << "{ Project onto axis: " << pm.project_to_axes.front() << "\n";
  int i = 0;
  for (auto const &[proj_extent, bins] : pm.bin_columns) {
    os << "  { projected bin: " << (i++) << ", extent: " << proj_extent
       << fmt::format(", original_bins: {} }}\n", bins);
  }
  return os << "}" << std::endl;
}

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> proj_to_axes) {
  auto const &pm = BuildProjectionMap(hf.binning, proj_to_axes);

  std::vector<std::string> labels;
  for (auto proj_to_axis : proj_to_axes) {
    labels.push_back(hf.binning.axis_labels[proj_to_axis]);
  }

  HistFrame projhf(Binning::from_extents(pm.projected_extents, labels));
  projhf.column_info = hf.column_info;
  projhf.reset();

  for (size_t row_i = 0; row_i < pm.projected_extents.size(); ++row_i) {
    for (auto const &bi_it : pm.bin_columns.at(pm.projected_extents[row_i])) {
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
