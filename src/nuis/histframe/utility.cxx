#include "nuis/histframe/utility.h"

#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "fmt/ranges.h"

#include "nuis/log.txx"

namespace nuis {

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
T Project_impl(T const &histlike, std::vector<size_t> const &proj_to_axes) {
  auto const &pm = BuildProjectionMap(histlike.binning, proj_to_axes);

  std::vector<std::string> labels;
  for (auto proj_to_axis : proj_to_axes) {
    labels.push_back(histlike.binning->axis_labels[proj_to_axis]);
  }

  T projhl(Binning::from_extents(pm.projected_extents, labels));
  projhl.column_info = histlike.column_info;
  projhl.resize();

  for (size_t row_i = 0; row_i < pm.projected_extents.size(); ++row_i) {
    for (Binning::index_t bi_it : pm.bin_columns[row_i]) {
      if constexpr (std::is_same_v<T, HistFrame>) {
        projhl.sumweights.row(row_i) += histlike.sumweights.row(bi_it);
        projhl.variances.row(row_i) += histlike.variances.row(bi_it);
      } else if (std::is_same_v<T, BinnedValues>) {
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

HistFrame Project(HistFrame const &hf,
                  std::vector<size_t> const &proj_to_axes) {
  return Project_impl<HistFrame>(hf, proj_to_axes);
}
HistFrame Project(HistFrame const &hf, size_t proj_to_axis) {
  return Project(hf, std::vector<size_t>{proj_to_axis});
}

BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes) {
  return Project_impl<BinnedValues>(hf, proj_to_axes);
}
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis) {
  return Project(hf, std::vector<size_t>{proj_to_axis});
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

} // namespace nuis
