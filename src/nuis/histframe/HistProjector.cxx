#include "nuis/histframe/HistProjector.h"
#include "nuis/histframe/BinningUtility.h"

#include "fmt/ranges.h"

#include "nuis/log.txx"

namespace nuis {

struct ProjectionMap {
  std::vector<size_t> project_to_axes;
  std::vector<Binning::BinExtents> projected_extents;
  std::vector<std::vector<Binning::Index>> bin_columns;
};

ProjectionMap BuildProjectionMap(Binning const &bin_info,
                                 std::vector<size_t> proj_to_axes) {
  ProjectionMap pm;
  pm.project_to_axes = proj_to_axes;
  pm.projected_extents = project_to_unique_bins(bin_info.bins, proj_to_axes);

  for (size_t i = 0; i < pm.projected_extents.size(); ++i) {
    pm.bin_columns.emplace_back(std::vector<Binning::Index>{});
  }

  for (Binning::Index bi_it = 0; bi_it < Binning::Index(bin_info.bins.size());
       ++bi_it) {

    auto const &bin = bin_info.bins[bi_it];

    Binning::BinExtents proj_bin;
    for (auto proj_to_axis : proj_to_axes) {
      proj_bin.push_back(bin[proj_to_axis]);
    }

    auto bin_it = std::find(pm.projected_extents.begin(),
                            pm.projected_extents.end(), proj_bin);

    if (bin_it == pm.projected_extents.end()) {
      log_critical(
          "[BuildProjectionMap]: When scanning bins, built projected bin "
          "extent that project_to_unique_bins did not find, this is a bug in "
          "NUISANCE, please report it to the authors.");
      std::stringstream ss;
      ss << "REPORT INFO:\n>>>----------------------------\ninput bin_info:\n"
         << bin_info << "\n";
      log_critical(ss.str());
      ss.str("");
      ss << "projected extents: " << pm.projected_extents << "\n";
      log_critical(ss.str());
      ss.str("");
      ss << "missed bin: " << proj_bin << "\n----------------------------<<<\n";
      throw CatastrophicBinningFailure();
    }
    pm.bin_columns[std::distance(pm.projected_extents.begin(), bin_it)]
        .push_back(bi_it);
  }

  return pm;
}

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::ProjectionMap const &pm) {
  os << "{ Project onto axis: " << pm.project_to_axes.front() << "\n";
  for (size_t i = 0; i < pm.projected_extents.size(); ++i) {
    os << "  { projected bin: " << (i)
       << ", extent: " << pm.projected_extents[i]
       << fmt::format(", original_bins: {} }}\n", pm.bin_columns[i]);
  }
  return os << "}" << std::endl;
}

namespace nuis {

HistFrame Project(HistFrame const &hf,
                  std::vector<size_t> const &proj_to_axes) {
  auto const &pm = BuildProjectionMap(hf.binning, proj_to_axes);

  std::vector<std::string> labels;
  for (auto proj_to_axis : proj_to_axes) {
    labels.push_back(hf.binning.axis_labels[proj_to_axis]);
  }

  HistFrame projhf(Binning::from_extents(pm.projected_extents, labels));
  projhf.column_info = hf.column_info;
  projhf.reset();

  for (size_t row_i = 0; row_i < pm.projected_extents.size(); ++row_i) {
    for (Binning::Index bi_it : pm.bin_columns[row_i]) {
      projhf.contents.row(row_i) += hf.contents.row(bi_it);
      projhf.variance.row(row_i) += hf.variance.row(bi_it);
    }
  }

  projhf.nfills = hf.nfills;

  return projhf;
}

HistFrame Project(HistFrame const &hf, size_t proj_to_axis) {
  return Project(hf, std::vector<size_t>{proj_to_axis});
}

} // namespace nuis
