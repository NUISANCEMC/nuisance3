#include "nuis/histframe/utility.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "fmt/ranges.h"

#include "nuis/log.txx"

namespace nuis {

NEW_NUISANCE_EXCEPT(InvalidAxisLabel);

struct ProjectionMap {
  std::vector<size_t> project_to_axes;
  std::vector<Binning::BinExtents> projected_extents;
  std::vector<std::vector<Binning::index_t>> bin_columns;
};

ProjectionMap BuildProjectionMap(BinningPtr const bin_info,
                                 std::vector<size_t> proj_to_axes) {
  ProjectionMap pm;
  pm.project_to_axes = proj_to_axes;
  pm.projected_extents = project_to_unique_bins(bin_info->bins, proj_to_axes);

  for (size_t i = 0; i < pm.projected_extents.size(); ++i) {
    pm.bin_columns.emplace_back(std::vector<Binning::index_t>{});
  }

  for (Binning::index_t bi_it = 0;
       bi_it < Binning::index_t(bin_info->bins.size()); ++bi_it) {

    auto const &bin = bin_info->bins[bi_it];

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

struct SliceMap {
  std::vector<Binning::index_t> bins_to_include;
  bool remove_sliced_axis;
};

SliceMap BuildSliceMap(BinningPtr const bin_info, size_t ax,
                       std::array<double, 2> slice_range,
                       bool exclude_range_end_bin) {

  SliceMap sm;
  // only remove the axis if there is more than one axis
  sm.remove_sliced_axis = bin_info->bins.front().size() > 1 ? true : false;

  SingleExtent firstext{0xdeadbeef, 0xdeadbeef};

  log_debug("[BuildSliceMap]<<<<<<<<<");
  log_debug("Slice: ax: {}, range: {}, exclude_range_end_bin: {}", ax,
            slice_range, exclude_range_end_bin);

  for (Binning::index_t bi_it = 0; bi_it < bin_info->bins.size(); ++bi_it) {
    log_debug("  bin {}", str_via_ss(bin_info->bins[bi_it]));

    // if we are given a single value and it is in a bin, include that bin
    if (!((slice_range[0] == slice_range[1]) &&
          bin_info->bins[bi_it][ax].contains(slice_range[0]))) {
      if ((bin_info->bins[bi_it][ax].high <= slice_range[0]) ||
          (bin_info->bins[bi_it][ax].low >= slice_range[1])) {
        log_debug("    excluded: {}, slice: {}",
                  str_via_ss(bin_info->bins[bi_it][ax]), slice_range);
        continue;
      }
      if (exclude_range_end_bin &&
          bin_info->bins[bi_it][ax].contains(slice_range[1])) {
        log_debug("    excluded: {}, slice_end: {}",
                  str_via_ss(bin_info->bins[bi_it][ax]), slice_range[1]);

        continue;
      }
    }

    if (firstext.low == 0xdeadbeef) {
      firstext = bin_info->bins[bi_it][ax];
    } else {
      if (!(firstext == bin_info->bins[bi_it][ax])) {
        sm.remove_sliced_axis = false;
      }
    }
    sm.bins_to_include.push_back(bi_it);
    log_debug("    included.");
  }
  log_debug("[BuildSliceMap]>>>>>>>>>");

  return sm;
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

template <typename T>
T Project_impl(T const &histlike, std::vector<size_t> const &proj_to_axes,
               bool result_has_binning) {
  auto const &pm = BuildProjectionMap(histlike.binning, proj_to_axes);

  std::vector<std::string> labels;
  for (auto proj_to_axis : proj_to_axes) {
    labels.push_back(histlike.binning->axis_labels[proj_to_axis]);
  }

  T projhl;

  if (result_has_binning) {
    projhl.binning = Binning::from_extents(pm.projected_extents, labels);
  } else { // make a binning that cannot be used to find a bin or fill useful
           // for expensive constructors
    projhl.binning = std::make_shared<nuis::Binning>();
    projhl.binning->axis_labels = labels;
    projhl.binning->bins = pm.projected_extents;
  }

  projhl.column_info = histlike.column_info;
  projhl.resize();

  for (size_t row_i = 0; row_i < pm.projected_extents.size(); ++row_i) {
    for (Binning::index_t bi_it : pm.bin_columns[row_i]) {
      if constexpr (std::is_same_v<T, HistFrame>) {
        projhl.sumweights.row(row_i) += histlike.sumweights.row(bi_it);
        projhl.variances.row(row_i) += histlike.variances.row(bi_it);
      } else if constexpr (std::is_same_v<T, BinnedValues>) {
        projhl.values.row(row_i) += histlike.values.row(bi_it);
        projhl.errors.row(row_i) += histlike.errors.row(bi_it).square();
      }
    }
  }
  if constexpr (std::is_same_v<T, BinnedValues>) {
    projhl.errors.sqrt();
  }

  if constexpr (std::is_same_v<T, HistFrame>) {
    projhl.num_fills = histlike.num_fills;
  }

  return projhl;
}

HistFrame Project(HistFrame const &hf, std::vector<size_t> const &proj_to_axes,
                  bool result_has_binning) {
  return Project_impl<HistFrame>(hf, proj_to_axes, result_has_binning);
}
HistFrame Project(HistFrame const &hf, size_t proj_to_axis,
                  bool result_has_binning) {
  return Project(hf, std::vector<size_t>{proj_to_axis}, result_has_binning);
}
HistFrame Project(HistFrame const &hf,
                  std::vector<std::string> const &proj_to_axes,
                  bool result_has_binning) {

  std::vector<size_t> proj_to_axes_idx;
  for (auto const &ax : proj_to_axes) {
    auto ax_it = std::find(hf.binning->axis_labels.begin(),
                           hf.binning->axis_labels.end(), ax);
    if (ax_it == hf.binning->axis_labels.end()) {
      throw InvalidAxisLabel() << "Project passed axis name: " << ax
                               << ", but no axis_label matches.";
    }
    proj_to_axes_idx.push_back(
        std::distance(hf.binning->axis_labels.begin(), ax_it));
  }

  return Project_impl<HistFrame>(hf, proj_to_axes_idx, result_has_binning);
}
HistFrame Project(HistFrame const &hf, std::string const &proj_to_axis,
                  bool result_has_binning) {
  return Project(hf, std::vector<std::string>{proj_to_axis},
                 result_has_binning);
}

BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes,
                     bool result_has_binning) {
  return Project_impl<BinnedValues>(hf, proj_to_axes, result_has_binning);
}
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis,
                     bool result_has_binning) {
  return Project(hf, std::vector<size_t>{proj_to_axis}, result_has_binning);
}
BinnedValues Project(BinnedValues const &hf,
                     std::vector<std::string> const &proj_to_axes,
                     bool result_has_binning) {

  std::vector<size_t> proj_to_axes_idx;
  for (auto const &ax : proj_to_axes) {
    auto ax_it = std::find(hf.binning->axis_labels.begin(),
                           hf.binning->axis_labels.end(), ax);
    if (ax_it == hf.binning->axis_labels.end()) {
      throw InvalidAxisLabel() << "Project passed axis name: " << ax
                               << ", but no axis_label matches.";
    }
    proj_to_axes_idx.push_back(
        std::distance(hf.binning->axis_labels.begin(), ax_it));
  }

  return Project_impl<BinnedValues>(hf, proj_to_axes_idx, result_has_binning);
}
BinnedValues Project(BinnedValues const &hf, std::string const &proj_to_axis,
                     bool result_has_binning) {
  return Project(hf, std::vector<std::string>{proj_to_axis},
                 result_has_binning);
}

template <typename T>
T Slice_impl(T const &histlike, size_t ax, std::array<double, 2> slice_range,
             bool exclude_range_end_bin, bool result_has_binning) {
  auto const &sm =
      BuildSliceMap(histlike.binning, ax, slice_range, exclude_range_end_bin);

  if (!sm.bins_to_include.size()) {
    log_critical("When slicing histogram along axes {}: {} ", ax, slice_range);
    log_critical("Kept no bins from original binning:\n{}",
                 str_via_ss(histlike.binning));
    throw EmptyBinning();
  }

  size_t nax = histlike.binning->bins.front().size();

  std::vector<std::string> labels;
  for (size_t ax_it = 0; ax_it < nax; ++ax_it) {
    if ((ax == ax_it) &&
        sm.remove_sliced_axis) { // skip the label if we're removing the axis
      continue;
    }
    labels.push_back(histlike.binning->axis_labels[ax_it]);
  }

  std::vector<Binning::BinExtents> sliced_extents;
  for (auto bi_it : sm.bins_to_include) {
    sliced_extents.emplace_back();
    for (size_t ax_it = 0; ax_it < nax; ++ax_it) {
      if ((ax == ax_it) &&
          sm.remove_sliced_axis) { // skip the extent if we're removing the axis
        continue;
      }
      sliced_extents.back().push_back(histlike.binning->bins[bi_it][ax_it]);
    }
  }

  T projhl;

  if (result_has_binning) {
    projhl.binning = Binning::from_extents(sliced_extents, labels);
  } else { // make a binning that cannot be used to find a bin or fill useful
           // for expensive constructors
    projhl.binning = std::make_shared<nuis::Binning>();
    projhl.binning->axis_labels = labels;
    projhl.binning->bins = sliced_extents;
  }

  projhl.column_info = histlike.column_info;
  projhl.resize();

  for (size_t new_bin_it = 0; new_bin_it < sm.bins_to_include.size();
       ++new_bin_it) {
    if constexpr (std::is_same_v<T, HistFrame>) {
      projhl.sumweights.row(new_bin_it) =
          histlike.sumweights.row(sm.bins_to_include[new_bin_it]);
      projhl.variances.row(new_bin_it) =
          histlike.variances.row(sm.bins_to_include[new_bin_it]);
    } else if constexpr (std::is_same_v<T, BinnedValues>) {
      projhl.values.row(new_bin_it) =
          histlike.values.row(sm.bins_to_include[new_bin_it]);
      projhl.errors.row(new_bin_it) =
          histlike.errors.row(sm.bins_to_include[new_bin_it]);
    }
  }

  if constexpr (std::is_same_v<T, HistFrame>) {
    projhl.num_fills = histlike.num_fills;
  }

  return projhl;
}

HistFrame Slice(HistFrame const &hf, size_t ax,
                std::array<double, 2> slice_range, bool exclude_range_end_bin,
                bool result_has_binning) {
  return Slice_impl<HistFrame>(hf, ax, slice_range, exclude_range_end_bin,
                               result_has_binning);
}
HistFrame Slice(HistFrame const &hf, size_t ax, double slice_val,
                bool result_has_binning) {
  return Slice_impl<HistFrame>(hf, ax, {slice_val, slice_val}, false,
                               result_has_binning);
}
HistFrame Slice(HistFrame const &hf, std::string const &ax,
                std::array<double, 2> slice_range, bool exclude_range_end_bin,
                bool result_has_binning) {
  auto ax_it = std::find(hf.binning->axis_labels.begin(),
                         hf.binning->axis_labels.end(), ax);
  if (ax_it == hf.binning->axis_labels.end()) {
    throw InvalidAxisLabel()
        << "Slice passed axis name: " << ax << ", but no axis_label matches.";
  }
  return Slice_impl<HistFrame>(
      hf, std::distance(hf.binning->axis_labels.begin(), ax_it), slice_range,
      exclude_range_end_bin, result_has_binning);
}
HistFrame Slice(HistFrame const &hf, std::string const &ax, double slice_val,
                bool result_has_binning) {
  auto ax_it = std::find(hf.binning->axis_labels.begin(),
                         hf.binning->axis_labels.end(), ax);
  if (ax_it == hf.binning->axis_labels.end()) {
    throw InvalidAxisLabel()
        << "Slice passed axis name: " << ax << ", but no axis_label matches.";
  }
  return Slice_impl<HistFrame>(
      hf, std::distance(hf.binning->axis_labels.begin(), ax_it),
      {slice_val, slice_val}, false, result_has_binning);
}
BinnedValues Slice(BinnedValues const &hf, size_t ax,
                   std::array<double, 2> slice_range,
                   bool exclude_range_end_bin, bool result_has_binning) {
  return Slice_impl<BinnedValues>(hf, ax, slice_range, exclude_range_end_bin,
                                  result_has_binning);
}
BinnedValues Slice(BinnedValues const &hf, size_t ax, double slice_val,
                   bool result_has_binning) {
  return Slice_impl<BinnedValues>(hf, ax, {slice_val, slice_val}, false,
                                  result_has_binning);
}

BinnedValues Slice(BinnedValues const &hf, std::string const &ax,
                   std::array<double, 2> slice_range,
                   bool exclude_range_end_bin, bool result_has_binning) {
  auto ax_it = std::find(hf.binning->axis_labels.begin(),
                         hf.binning->axis_labels.end(), ax);
  if (ax_it == hf.binning->axis_labels.end()) {
    throw InvalidAxisLabel()
        << "Slice passed axis name: " << ax << ", but no axis_label matches.";
  }
  return Slice_impl<BinnedValues>(
      hf, std::distance(hf.binning->axis_labels.begin(), ax_it), slice_range,
      exclude_range_end_bin, result_has_binning);
}
BinnedValues Slice(BinnedValues const &hf, std::string const &ax,
                   double slice_val, bool result_has_binning) {
  auto ax_it = std::find(hf.binning->axis_labels.begin(),
                         hf.binning->axis_labels.end(), ax);
  if (ax_it == hf.binning->axis_labels.end()) {
    throw InvalidAxisLabel()
        << "Slice passed axis name: " << ax << ", but no axis_label matches.";
  }
  return Slice_impl<BinnedValues>(
      hf, std::distance(hf.binning->axis_labels.begin(), ax_it),
      {slice_val, slice_val}, false, result_has_binning);
}

BinnedValues Add(BinnedValues const &hf1, BinnedValues const &hf2) {
  BinnedValues out(hf1);
  out.values += hf2.values;
  out.errors = (hf1.values.square() + hf2.values.square()).sqrt();
  return out;
}

HistFrame Add(HistFrame const &hf1, HistFrame const &hf2) {
  HistFrame out(hf1);
  out.sumweights += hf2.sumweights;
  out.variances = hf1.variances + hf2.variances;
  return out;
}

BinnedValues Scale(BinnedValues const &hf, double factor) {
  BinnedValues out(hf);
  out.values *= factor;
  out.errors *= factor;
  return out;
}

HistFrame Scale(HistFrame const &hf, double factor) {
  HistFrame out(hf);
  out.sumweights *= factor;
  out.variances *= factor * factor;
  return out;
}

BinnedValues Multiply(BinnedValues const &hf1, BinnedValues const &hf2) {
  BinnedValues out(hf1);
  out.values *= hf2.values;
  out.errors = out.values * ((hf1.errors / hf1.values).square() +
                             (hf2.errors / hf2.values).square())
                                .sqrt();
  return out;
}

HistFrame Multiply(HistFrame const &hf1, HistFrame const &hf2) {
  HistFrame out(hf1);
  out.sumweights *= hf2.sumweights;
  out.variances =
      out.sumweights.square() * ((hf1.variances / hf1.sumweights.square()) +
                                 (hf2.variances / hf2.sumweights.square()));
  return out;
}

BinnedValues Divide(BinnedValues const &hf_num, BinnedValues const &hf_denom) {
  BinnedValues out(hf_num);
  out.values /= hf_denom.values;
  out.errors = out.values * ((hf_num.errors / hf_num.values).square() +
                             (hf_denom.errors / hf_denom.values).square())
                                .sqrt();
  return out;
}

HistFrame Divide(HistFrame const &hf_num, HistFrame const &hf_denom) {
  HistFrame out(hf_num);
  out.sumweights /= hf_denom.sumweights;
  out.variances = out.sumweights.square() *
                  ((hf_num.variances / hf_num.sumweights.square()) +
                   (hf_denom.variances / hf_denom.sumweights.square()));
  return out;
}

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesBase const &bvb) {

  size_t abs_max_width = 12;

  auto contents = bvb.get_bin_contents();
  auto errors = bvb.get_bin_uncertainty();

  std::vector<size_t> col_widths(contents.cols() * 2, 0);

  // check up to the first 20 rows to guess how wide we need each column
  for (int ri = 0; ri < contents.rows(); ++ri) {
    for (int ci = 0; ci < (contents.cols() * 2); ++ci) {

      double v = (ci & 1) ? errors(ri, ci / 2) : (contents(ri, ci / 2));
      std::string test = fmt::format("{:>.4}", v);

      size_t len = test.size() - test.find_first_not_of(" ");
      col_widths[ci] = std::min(std::max(col_widths[ci], len), abs_max_width);
    }
    if (ri >= 20) {
      break;
    }
  }

  std::stringstream hdr;
  std::vector<std::string> fmtstrs;
  hdr << " | bin |";

  for (size_t ci = 0; ci < (bvb.column_info.size() * 2); ++ci) {
    std::string cfull =
        (ci & 1) ? std::string("err") : bvb.column_info[ci / 2].name;
    std::string cn = (cfull.size() > abs_max_width)
                         ? cfull.substr(0, abs_max_width - 1) + "$"
                         : cfull;

    col_widths[ci] = std::max(col_widths[ci], cn.size());

    hdr << fmt::format(" {:>" + std::to_string(col_widths[ci]) + "} |", cn);
    fmtstrs.push_back(" {:>" + std::to_string(col_widths[ci]) + ".4} |");
  }

  std::string hdrs = hdr.str();

  std::vector<char> line(hdrs.size() + 1, '-');
  line[hdrs.size() - 1] = '\0';
  os << " " << line.data() << std::endl;
  os << hdrs << std::endl;
  os << " " << line.data() << std::endl;

  for (int ri = 0; ri < contents.rows(); ++ri) {
    os << fmt::format(" | {:>3} |", ri);
    for (int ci = 0; ci < (contents.cols() * 2); ++ci) {
      double v = (ci & 1) ? errors(ri, ci / 2) : contents(ri, ci / 2);
      os << fmt::format(fmtstrs[ci], v);
    }
    os << std::endl;
    if (ri >= 20) {
      os << " |";
      for (int ci = 0; ci < (contents.cols() * 2); ++ci) {
        os << fmt::format(fmtstrs[ci], "...");
      }
      os << std::endl;
      break;
    }
  }

  return os << " " << line.data();
}

BinnedValues ToCountDensity(BinnedValues const &bv) {
  BinnedValues out = bv;

  auto bin_sizes = bv.binning->bin_sizes();
  out.values.colwise() /= bin_sizes;
  out.errors.colwise() /= bin_sizes;

  return out;
}
BinnedValues ToCount(BinnedValues const &bv) {
  BinnedValues out = bv;

  auto bin_sizes = bv.binning->bin_sizes();
  out.values.colwise() *= bin_sizes;
  out.errors.colwise() *= bin_sizes;

  return out;
}

} // namespace nuis
