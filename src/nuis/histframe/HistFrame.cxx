#include "nuis/histframe/HistFrame.h"

#include "nuis/binning/exceptions.h"

#include "nuis/log.txx"

#include "fmt/ranges.h"

#include "nuis/histframe/utility.h"

NEW_NUISANCE_EXCEPT(MissingProjectionEncountered);
NEW_NUISANCE_EXCEPT(InvalidColumnAccess);

namespace nuis {

HistFrame::HistFrame(BinningPtr binop, std::string const &def_col_name,
                     std::string const &def_col_label)
    : column_info{{def_col_name, def_col_label}}, binning(binop) {
  reset();
  bin_weights = Eigen::ArrayXd::Constant(binning->bins.size(), 1);
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

Eigen::ArrayXd HistFrame::get_values(HistFrame::column_t colid) const {
  if (colid >= contents.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, contents.cols());
    throw InvalidColumnAccess();
  }
  return (contents.col(colid) * bin_weights);
}
Eigen::ArrayXd HistFrame::get_errors(HistFrame::column_t colid) const {
  if (colid >= variance.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, variance.cols());
    throw InvalidColumnAccess();
  }
  return variance.col(colid).sqrt() * bin_weights;
}

HistFrame::column_valerr
HistFrame::get_column(HistFrame::column_t colid) const {
  if (colid >= contents.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, contents.cols());
    throw InvalidColumnAccess();
  }
  return {get_values(colid), get_errors(colid)};
}

HistFrame::column_view HistFrame::operator[](HistFrame::column_t colid) {
  if (colid >= contents.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, contents.cols());
    throw InvalidColumnAccess();
  }
  return {contents.col(colid), variance.col(colid), bin_weights};
}

HistFrame::column_view HistFrame::operator[](std::string const &name) {
  auto colid = find_column_index(name);
  if (colid == npos) {
    log_critical("Tried to access non-existant column with name {}", name);
    throw InvalidColumnAccess();
  }
  return operator[](colid);
}

Binning::index_t
HistFrame::find_bin(std::vector<double> const &projections) const {
  if (projections.size() < binning->number_of_axes()) {
    NUIS_LOG_DEBUG("Too few projections passed to HistFrame::fill: {}. Compile "
                   "with CMAKE_BUILD_TYPE=Debug to make this an exception.",
                   projections);
#ifndef NUIS_NDEBUG
    throw MismatchedAxisCount();
#endif
    return npos;
  }

  // can't search too far as we use a possibly oversized static local vector
  auto end = projections.begin() + binning->number_of_axes();
  if (std::find(projections.begin(), end, HistFrame::missing_datum) != end) {
    NUIS_LOG_DEBUG(
        "Found missing_datum flag in projection vector passed to fill: {}",
        projections);
#ifndef NUIS_NDEBUG
    throw MissingProjectionEncountered();
#endif
    return npos;
  }

  return binning->find_bin(projections);
}

Binning::index_t HistFrame::find_bin(double proj) const {
  return find_bin(std::vector<double>{
      proj,
  });
}

void HistFrame::fill_bin(Binning::index_t i, double weight,
                         HistFrame::column_t col) {

#ifndef NDEBUG
  if (i == Binning::npos) {
    log_info("Tried to Fill histogram with out of range nuis::Binning::npos.");
    return;
  }
  if (i >= contents.rows()) {
    log_info("Tried to Fill histogram with out of range bin {}.", i);
    return;
  }
  if ((weight != 0) && (!std::isnormal(weight))) {
    log_warn("Tried to Fill histogram with a non-normal weight: {}.", weight);
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

void HistFrame::fill_with_selection(int sel_int,
                                    std::vector<double> const &projections,
                                    double weight, column_t col) {

  static std::vector<double> local_projections(10);
  local_projections.clear();
  local_projections.push_back(sel_int);
  std::copy(projections.begin(), projections.end(),
            std::back_inserter(local_projections));

  fill(local_projections, weight, col);
}

void HistFrame::fill_with_selection(int sel_int, double projection,
                                    double weight, column_t col) {
  fill_with_selection(sel_int, std::vector<double>{projection}, weight, col);
}

void HistFrame::fill(std::vector<double> const &projections, double weight,
                     HistFrame::column_t col) {
  fill_bin(find_bin(projections), weight, col);
}

void HistFrame::fill(double projection, double weight,
                     HistFrame::column_t col) {
  fill(std::vector<double>{projection}, weight, col);
}

void HistFrame::set_value_is_content_density() {
  bin_weights =
      Eigen::ArrayXd::Constant(binning->bins.size(), 1) / binning->bin_sizes();
}

void HistFrame::set_value_is_content() {
  bin_weights = Eigen::ArrayXd::Constant(binning->bins.size(), 1);
}

void HistFrame::reset() {
  contents = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  variance = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  nfills = 0;
}

std::ostream &operator<<(std::ostream &os, nuis::HistFramePrinter fp) {

  size_t abs_max_width = fp.max_col_width;

  auto const &f = fp.fr.get();

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
  hdr << " | bin |";

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
    os << fmt::format(" | {:>3} |", ri);
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
}

std::ostream &operator<<(std::ostream &os, nuis::HistFrame const &f) {
  return os << nuis::HistFramePrinter(f);
}

} // namespace nuis
