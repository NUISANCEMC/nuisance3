#include "nuis/histframe/HistFrame.h"

#include "spdlog/spdlog.h"

#include "fmt/ranges.h"

#include "nuis/histframe/Utility.h"

namespace nuis {

HistFrame::HistFrame(Bins::BinOp bindef)
    : binning(bindef), column_info{{"data", ""}, {"mc", ""}} {

  Reset();
}

HistFrame::column_t HistFrame::AddColumn(std::string const &name,
                                         std::string const &label) {
  column_info.emplace_back(ColumnInfo{name, label});

  Eigen::ArrayXXd content_copy = content, variance_copy = variance;
  Reset();
  content.leftCols(content_copy.cols()) = content_copy;
  variance.leftCols(variance_copy.cols()) = variance_copy;
  return HistFrame::column_t(column_info.size() - 1);
}

HistFrame::column_t HistFrame::GetColumnIndex(std::string const &name) const {
  for (size_t i = 0; i < column_info.size(); ++i) {
    if (name == column_info[i].name) {
      return HistFrame::column_t(i);
    }
  }
  spdlog::critical("Tried to get column, named {} from HistFrame. But no such "
                   "column exists.",
                   name);
  abort();
}

Bins::BinId HistFrame::FindBin(std::vector<double> const &projections) const {
  return binning.bin_func(projections);
}
void HistFrame::Fill(std::vector<double> const &projections, double weight,
                     HistFrame::column_t col) {
  Bins::BinId i = FindBin(projections);
#ifndef NDEBUG
  if (i == Bins::npos) {
    spdlog::critical(
        "Tried to Fill histogram with out of range nuis::Bins::npos.");
    abort();
    return;
  }
#endif
  content(i, col) += weight;
  variance(i, col) += weight * weight;
  nfills++;
}

void HistFrame::Fill(double proj, double weight, HistFrame::column_t col) {
  Fill(
      std::vector<double>{
          proj,
      },
      weight, col);
}

void HistFrame::ScaleColumn(double s, HistFrame::column_t col,
                            bool divide_by_cell_area) {
  for (int ri = 0; ri < content.rows(); ++ri) {
    double area = 1;
    if (divide_by_cell_area) {
      for (auto const &binrange : binning.bin_info.extents[ri]) {
        area *= binrange.width();
      }
    }

    content(ri, col) *= s / area;
    variance(ri, col) *= std::pow(s / area, 2);
  }
}
void HistFrame::MultiplyColumn(Eigen::ArrayXd const &other, HistFrame::column_t col) {
  content.col(col) *= other;
}
void HistFrame::DivideColumn(Eigen::ArrayXd const &other, HistFrame::column_t col) {
  content.col(col) /= other;
}
double HistFrame::ColumnIntegral(HistFrame::column_t col,
                                 bool multiply_by_cell_area) const {
  double integral = 0;
  for (int ri = 0; ri < content.rows(); ++ri) {
    double area = 1;
    if (multiply_by_cell_area) {
      for (auto const &binrange : binning.bin_info.extents[ri]) {
        area *= binrange.width();
      }
    }
    integral += content(ri, col) * area;
  }
  return integral;
}
void HistFrame::Reset() {
  content = Eigen::ArrayXXd::Zero(binning.bin_info.extents.size(),
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

    std::vector<size_t> col_widths(f.content.cols() * 2, 0);

    // check up to the first 20 rows to guess how wide we need each column
    for (int ri = 0; ri < f.content.rows(); ++ri) {
      for (int ci = 0; ci < (f.content.cols() * 2); ++ci) {

        double v = (ci & 1) ? std::sqrt(f.variance(ri, ci / 2))
                            : (f.content(ri, ci / 2));
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

    for (int ri = 0; ri < f.content.rows(); ++ri) {
      os << " |";
      for (int ci = 0; ci < (f.content.cols() * 2); ++ci) {
        double v = (ci & 1) ? std::sqrt(f.variance(ri, ci / 2))
                            : (f.content(ri, ci / 2));
        os << fmt::format(fmtstrs[ci], v);
      }
      os << std::endl;
      if (ri >= fp.max_rows) {
        os << " |";
        for (int ci = 0; ci < (f.content.cols() * 2); ++ci) {
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
  return os << f.content.topRows(fp.max_rows);
}

std::ostream &operator<<(std::ostream &os, nuis::HistFrame const &f) {
  return os << nuis::HistFramePrinter(f);
}