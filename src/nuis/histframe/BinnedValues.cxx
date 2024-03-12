#include "nuis/histframe/BinnedValues.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/frame/missing_datum.h"

#include "nuis/binning/exceptions.h"

#include "nuis/log.txx"

#include "fmt/ranges.h"

namespace nuis {

BinnedValues::BinnedValues(BinningPtr binop, std::string const &def_col_name,
                           std::string const &def_col_label)
    : binning(binop), column_info{{def_col_name, def_col_label}} {
  values = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  errors = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
}

BinnedValues::column_t BinnedValues::add_column(std::string const &name,
                                                std::string const &label) {
  column_info.emplace_back(ColumnInfo{name, label});

  Eigen::ArrayXXd content_copy = values, errors_copy = errors;

  values = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  errors = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());

  values.leftCols(content_copy.cols()) = content_copy;
  errors.leftCols(errors_copy.cols()) = errors_copy;

  return BinnedValues::column_t(column_info.size() - 1);
}

BinnedValues::column_t
BinnedValues::find_column_index(std::string const &name) const {
  for (size_t i = 0; i < column_info.size(); ++i) {
    if (name == column_info[i].name) {
      return BinnedValues::column_t(i);
    }
  }
  return BinnedValues::npos;
}

BinnedValues::column_view
BinnedValues::operator[](BinnedValues::column_t colid) {
  if (colid >= values.cols()) {
    log_critical("Tried to access column {}, but this BinnedValues instance "
                 "only has {} columns.",
                 colid, values.cols());
    throw InvalidColumnAccess();
  }
  return {values.col(colid), errors.col(colid)};
}

BinnedValues::column_view BinnedValues::operator[](std::string const &name) {
  auto colid = find_column_index(name);
  if (colid == npos) {
    log_critical("Tried to access non-existant column with name {}", name);
    throw InvalidColumnAccess();
  }
  return operator[](colid);
}

BinnedValues::column_view_const
BinnedValues::operator[](BinnedValues::column_t colid) const {
  if (colid >= values.cols()) {
    log_critical("Tried to access column {}, but this BinnedValues instance "
                 "only has {} columns.",
                 colid, values.cols());
    throw InvalidColumnAccess();
  }
  return {values.col(colid), errors.col(colid)};
}

BinnedValues::column_view_const
BinnedValues::operator[](std::string const &name) const {
  auto colid = find_column_index(name);
  if (colid == npos) {
    log_critical("Tried to access non-existant column with name {}", name);
    throw InvalidColumnAccess();
  }
  return operator[](colid);
}

Binning::index_t
BinnedValues::find_bin(std::vector<double> const &projections) const {
  if (projections.size() < binning->number_of_axes()) {
    NUIS_LOG_DEBUG(
        "Too few projections passed to BinnedValues::find_bin: {}. Compile "
        "with CMAKE_BUILD_TYPE=Debug to make this an exception.",
        projections);
#ifndef NUIS_NDEBUG
    throw MismatchedAxisCount();
#endif
    return npos;
  }

  // can't search too far as we use a possibly oversized static local vector
  auto end = projections.begin() + binning->number_of_axes();
  if (std::find(projections.begin(), end, kMissingDatum) != end) {
    NUIS_LOG_DEBUG("Found kMissingDatum flag in projection vector passed to "
                   "BinnedValues::find_bin: {}",
                   projections);
#ifndef NUIS_NDEBUG
    throw MissingProjectionEncountered();
#endif
    return npos;
  }

  return binning->find_bin(projections);
}

Binning::index_t BinnedValues::find_bin(double proj) const {
  static std::vector<double> dummy = {0};
  dummy[0] = proj;
  return find_bin(dummy);
}

HistFrame BinnedValues::make_HistFrame(column_t col) const {
  if (col != npos) {
    return {binning, column_info[col].name,
            column_info[col].dependent_axis_label};
  }
  HistFrame hf(binning, column_info[0].name,
               column_info[0].dependent_axis_label);
  for (size_t i = 1; i < column_info.size(); ++i) {
    hf.add_column(column_info[0].name, column_info[0].dependent_axis_label);
  }
  return hf;
}

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesPrinter fp) {

  size_t abs_max_width = fp.max_col_width;

  auto const &f = fp.fr.get();

  std::vector<size_t> col_widths(f.values.cols() * 2, 0);

  // check up to the first 20 rows to guess how wide we need each column
  for (int ri = 0; ri < f.values.rows(); ++ri) {
    for (int ci = 0; ci < (f.values.cols() * 2); ++ci) {

      double v =
          (ci & 1) ? std::sqrt(f.errors(ri, ci / 2)) : (f.values(ri, ci / 2));
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

  for (int ri = 0; ri < f.values.rows(); ++ri) {
    os << fmt::format(" | {:>3} |", ri);
    for (int ci = 0; ci < (f.values.cols() * 2); ++ci) {
      double v = (ci & 1) ? f.errors(ri, ci / 2) : (f.values(ri, ci / 2));
      os << fmt::format(fmtstrs[ci], v);
    }
    os << std::endl;
    if (ri >= fp.max_rows) {
      os << " |";
      for (int ci = 0; ci < (f.values.cols() * 2); ++ci) {
        os << fmt::format(fmtstrs[ci], "...");
      }
      os << std::endl;
      break;
    }
  }

  return os << " " << line.data();
}

std::ostream &operator<<(std::ostream &os, nuis::BinnedValues const &f) {
  return os << nuis::BinnedValuesPrinter(f);
}

} // namespace nuis
