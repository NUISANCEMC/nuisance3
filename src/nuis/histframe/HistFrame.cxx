#include "nuis/histframe/HistFrame.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

#include "nuis/histframe/Utility.h"

namespace nuis {

HistFrame::HistFrame(Bins::BinOp bindef, std::string const &def_col_name,
                     std::string const &def_col_label)
    : binning(bindef), column_info{{def_col_name, def_col_label}} {

  reset();
}

HistFrame::column_t HistFrame::add_column(std::string const &name,
                                          std::string const &label) {
  column_info.emplace_back(ColumnInfo{name, label});

  Eigen::ArrayXXd content_copy = contents, variance_copy = variance;
  reset();
  contents.leftCols(content_copy.cols()) = content_copy;
  variance.leftCols(variance_copy.cols()) = variance_copy;
  return HistFrame::column_t(column_info.size() - 1);
}

HistFrame::column_t
HistFrame::find_column_index(std::string const &name) const {
  for (size_t i = 0; i < column_info.size(); ++i) {
    if (name == column_info[i].name) {
      return HistFrame::column_t(i);
    }
  }
  return HistFrame::npos;
}
Eigen::ArrayXd HistFrame::get_content(HistFrame::column_t col,
                                      bool divide_by_bin_sizes) const {
  return divide_by_bin_sizes
             ? (contents.col(col) / binning.bin_info.bin_sizes()).eval()
             : contents.col(col);
}
Eigen::ArrayXd HistFrame::get_error(HistFrame::column_t col,
                                    bool divide_by_bin_sizes) const {
  return (divide_by_bin_sizes
              ? (contents.col(col) / (binning.bin_info.bin_sizes().square()))
                    .eval()
              : contents.col(col))
      .sqrt();
}

Bins::BinId HistFrame::find_bin(std::vector<double> const &projections) const {
  return binning.bin_func(projections);
}
Bins::BinId HistFrame::find_bin(double proj) const {
  return find_bin(std::vector<double>{
      proj,
  });
}

void HistFrame::fill_bin(Bins::BinId i, double weight,
                         HistFrame::column_t col) {
#ifndef NDEBUG
  if (i == Bins::npos) {
    spdlog::critical(
        "Tried to Fill histogram with out of range nuis::Bins::npos.");
    return;
  }
  if (i >= contents.rows()) {
    spdlog::critical("Tried to Fill histogram with out of range bin {}.", i);
    return;
  }
  if ((weight != 0) && (!std::isnormal(weight))) {
    spdlog::critical("Tried to Fill histogram with a non-normal weight: {}.",
                     weight);
    return;
  }
#endif

  if ((i >= contents.rows()) || !std::isnormal(weight)) {
    return;
  }

  contents(i, col) += weight;
  variance(i, col) += weight * weight;
  nfills++;
}

void HistFrame::fill(std::vector<double> const &projections, double weight,
                     HistFrame::column_t col) {
  fill_bin(find_bin(projections), weight, col);
}

void HistFrame::fill(double proj, double weight, HistFrame::column_t col) {
  fill(
      std::vector<double>{
          proj,
      },
      weight, col);
}

void HistFrame::reset() {
  contents = Eigen::ArrayXXd::Zero(binning.bin_info.extents.size(),
                                   column_info.size());
  variance = Eigen::ArrayXXd::Zero(binning.bin_info.extents.size(),
                                   column_info.size());
  nfills = 0;
}
} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::HistFramePrinter fp) {

  size_t abs_max_width = fp.max_col_width;

  auto const &f = fp.fr.get();

  if (fp.format == "table") {

    std::vector<size_t> col_widths(f.contents.cols() * 2, 0);

    // check up to the first 20 rows to guess how wide we need each column
    for (int ri = 0; ri < f.contents.rows(); ++ri) {
      for (int ci = 0; ci < (f.contents.cols() * 2); ++ci) {

        double v = (ci & 1) ? std::sqrt(f.variance(ri, ci / 2))
                            : (f.contents(ri, ci / 2));
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
    hdr << " |";

    for (size_t ci = 0; ci < (f.column_info.size() * 2); ++ci) {
      std::string cfull =
          (ci & 1) ? std::string("err") : f.column_info[ci / 2].name;
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

    for (int ri = 0; ri < f.contents.rows(); ++ri) {
      os << " |";
      for (int ci = 0; ci < (f.contents.cols() * 2); ++ci) {
        double v = (ci & 1) ? std::sqrt(f.variance(ri, ci / 2))
                            : (f.contents(ri, ci / 2));
        os << fmt::format(fmtstrs[ci], v);
      }
      os << std::endl;
      if (ri >= fp.max_rows) {
        os << " |";
        for (int ci = 0; ci < (f.contents.cols() * 2); ++ci) {
          os << fmt::format(fmtstrs[ci], "...");
        }
        os << std::endl;
        break;
      }
    }

    return os << " " << line.data();
  } else if (fp.format == "json") {
    return os << boost::json::serialize(boost::json::value_from(f));
  }
  return os << f.contents.topRows(fp.max_rows);
}

std::ostream &operator<<(std::ostream &os, nuis::HistFrame const &f) {
  return os << nuis::HistFramePrinter(f);
}