#include "nuis/histframe/HistFrame.h"

#include "nuis/eventframe/missing_datum.h"

#include "nuis/binning/exceptions.h"

#include "nuis/log.txx"

#include "fmt/ranges.h"

NEW_NUISANCE_EXCEPT(InvalidColumnAccess);

namespace nuis {

HistFrame::column_view HistFrame::operator[](HistFrame::column_t colid) {
  if (colid >= sumweights.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, sumweights.cols());
    throw InvalidColumnAccess();
  }
  return {sumweights.col(colid), variances.col(colid)};
}

HistFrame::column_view HistFrame::operator[](std::string const &name) {
  auto colid = find_column_index(name);
  if (colid == npos) {
    log_critical("Tried to access non-existant column with name {}", name);
    throw InvalidColumnAccess();
  }
  return operator[](colid);
}

HistFrame::column_view_const
HistFrame::operator[](HistFrame::column_t colid) const {
  if (colid >= sumweights.cols()) {
    log_critical(
        "Tried to access column {}, but this HistFrame only has {} columns.",
        colid, sumweights.cols());
    throw InvalidColumnAccess();
  }
  return {sumweights.col(colid), variances.col(colid)};
}

HistFrame::column_view_const
HistFrame::operator[](std::string const &name) const {
  auto colid = find_column_index(name);
  if (colid == npos) {
    log_critical("Tried to access non-existant column with name {}", name);
    throw InvalidColumnAccess();
  }
  return operator[](colid);
}

void HistFrame::fill_bin(Binning::index_t i, double weight,
                         HistFrame::column_t col) {

#ifndef NDEBUG
  if (i == Binning::npos) {
    log_info("Tried to Fill histogram with out of range nuis::Binning::npos.");
    return;
  }
  if (i >= sumweights.rows()) {
    log_info("Tried to Fill histogram with out of range bin {}.", i);
    return;
  }
  if ((weight != 0) && (!std::isnormal(weight))) {
    log_warn("Tried to Fill histogram with a non-normal weight: {}.", weight);
    return;
  }
#endif

  if ((i >= sumweights.rows()) || !std::isnormal(weight)) {
    return;
  }

  sumweights(i, col) += weight;
  variances(i, col) += weight * weight;
  num_fills++;
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
  static std::vector<double> dummy = {0, 0};
  dummy[0] = sel_int;
  dummy[1] = projection;
  fill(dummy, weight, col);
}

void HistFrame::fill(std::vector<double> const &projections, double weight,
                     HistFrame::column_t col) {
  fill_bin(find_bin(projections), weight, col);
}

void HistFrame::fill(double projection, double weight,
                     HistFrame::column_t col) {
  static std::vector<double> dummy = {0};
  dummy[0] = projection;
  fill(dummy, weight, col);
}

BinnedValues HistFrame::finalise(bool divide_by_bin_sizes) const {
  BinnedValues bv;

  bv.binning = binning;
  for (auto const &[name, dal] : column_info) {
    bv.add_column(name, dal);
  }

  bv.values = sumweights;
  bv.errors = variances.sqrt();

  if (divide_by_bin_sizes) {
    auto bin_sizes = binning->bin_sizes();
    bv.values /= bin_sizes;
    bv.errors /= bin_sizes;
  }

  return bv;
}

void HistFrame::reset() {
  sumweights = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  variances = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  num_fills = 0;
}

void HistFrame::resize() {
  if (sumweights.rows() < int(binning->bins.size())) {
    sumweights =
        Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
    variances = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
  }

  if (sumweights.cols() < int(column_info.size())) {
    Eigen::ArrayXXd content_copy = sumweights, errors_copy = variances;

    sumweights =
        Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());
    variances = Eigen::ArrayXXd::Zero(binning->bins.size(), column_info.size());

    sumweights.leftCols(content_copy.cols()) = content_copy;
    variances.leftCols(errors_copy.cols()) = errors_copy;
  }
}

} // namespace nuis
