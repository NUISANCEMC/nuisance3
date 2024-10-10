#include "nuis/histframe/BinnedValues.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/eventframe/missing_datum.h"

#include "nuis/binning/exceptions.h"

#include "nuis/log.txx"

#include "spdlog/fmt/bundled/ranges.h"

namespace nuis {

BinnedValuesBase::column_t
BinnedValuesBase::add_column(std::string const &name,
                             std::string const &label) {
  column_info.emplace_back(ColumnInfo{name, label});

  resize();

  return BinnedValuesBase::column_t(column_info.size() - 1);
}

BinnedValuesBase::column_t
BinnedValuesBase::find_column_index(std::string const &name) const {
  for (size_t i = 0; i < column_info.size(); ++i) {
    if (name == column_info[i].name) {
      return BinnedValuesBase::column_t(i);
    }
  }
  return BinnedValuesBase::npos;
}

Binning::index_t
BinnedValuesBase::find_bin(std::vector<double> const &projections) const {
  if (projections.size() < binning->number_of_axes()) {
    NUIS_LOG_DEBUG(
        "Too few projections passed to BinnedValuesBase::find_bin: {}. Compile "
        "with CMAKE_BUILD_TYPE=Debug to make this an exception.",
        projections);
#ifndef NUIS_NDEBUG
    throw MismatchedAxisCount();
#endif
    return npos;
  }

  // can't search too far as we use a possibly oversized static local vector
  auto end = projections.begin() + binning->number_of_axes();
  if (std::find(projections.begin(), end, kMissingDatum<double>) != end) {
#ifndef NUIS_NDEBUG
    NUIS_LOG_CRITICAL("Found kMissingDatum flag in projection vector passed to "
                      "BinnedValuesBase::find_bin: {}",
                      projections);
    throw MissingProjectionEncountered();
#endif
    return npos;
  }

  return binning->find_bin(projections);
}

Binning::index_t BinnedValuesBase::find_bin(double proj) const {
  static std::vector<double> dummy = {0};
  dummy[0] = proj;
  return find_bin(dummy);
}

BinnedValuesBase::BinnedValuesBase(BinningPtr binop,
                                   std::string const &def_col_name,
                                   std::string const &def_col_label)
    : binning(binop), column_info{{def_col_name, def_col_label}} {}

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

HistFrame BinnedValues::make_HistFrame(column_t col) const {
  if (col != npos) {
    return {binning, column_info[col].name, column_info[col].column_label};
  }
  HistFrame hf(binning, column_info[0].name, column_info[0].column_label);
  for (size_t i = 1; i < column_info.size(); ++i) {
    hf.add_column(column_info[0].name, column_info[0].column_label);
  }
  return hf;
}

void BinnedValues::resize() {
  if (values.rows() < int(binning->bins.size())) {
    values = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
    errors = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  }

  if (values.cols() < int(column_info.size())) {
    Eigen::ArrayXXd content_copy = values, errors_copy = errors;

    values = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
    errors = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());

    values.leftCols(content_copy.cols()) = content_copy;
    errors.leftCols(errors_copy.cols()) = errors_copy;
  }
}

} // namespace nuis
