#pragma once

#include "nuis/binning/Binning.h"

#include "nuis/eventframe/utility.h"

#include "Eigen/Dense"

#include <iostream>

namespace nuis {
std::vector<Binning::BinExtents> unique(std::vector<Binning::BinExtents> bins);
bool bins_overlap(Binning::BinExtents const &a, Binning::BinExtents const &b);
std::vector<Binning::BinExtents>
project_to_unique_bins(std::vector<Binning::BinExtents> const &bins,
                       std::vector<size_t> const &proj_to_axes);
bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          std::vector<size_t> const &proj_to_axes = {});
bool binning_has_overlaps(std::vector<Binning::BinExtents> const &bins,
                          size_t proj_to_axis);

bool is_contiguous(std::vector<Binning::BinExtents> const &bins);
std::vector<std::vector<double>>
get_contiguous_axes(std::vector<Binning::BinExtents> const &bins);

std::vector<double>
get_bin_edges1D(std::vector<Binning::BinExtents> const &bins, size_t ax = 0);

std::vector<std::vector<double>>
get_bin_centers(std::vector<Binning::BinExtents> const &bins);
std::vector<double>
get_bin_centers1D(std::vector<Binning::BinExtents> const &bins);

std::array<std::vector<double>, 2>
get_rectilinear_grid(std::vector<Binning::BinExtents> const &bins);

std::vector<double> log10_spaced_edges(double start, double stop, size_t nbins);
std::vector<double> ln_spaced_edges(double start, double stop, size_t nbins);

std::vector<double> uniform_width_edges(double start, double width,
                                        size_t nbins);

std::vector<double> lin_spaced_edges(double start, double stop, size_t nbins);

std::vector<double>
cat_bin_edges(std::vector<std::vector<double>> const &edge_series);

std::vector<Binning::BinExtents>
edges_to_extents(std::vector<double> const &edges);

std::vector<std::vector<Binning::index_t>>
get_sorted_bin_map(std::vector<Binning::BinExtents> const &bins);

std::pair<size_t, std::vector<std::vector<Binning::index_t>>>
get_bin_columns(std::vector<Binning::BinExtents> const &bins);

template <typename EFT>
inline Eigen::ArrayXi find_bins(BinningPtr &binning, EFT const &ef,
                         std::vector<std::string> const &projection_columns) {

#ifdef NUIS_ARROW_ENABLED
  using col_accessor_type = std::conditional_t<
      std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>,
      std::vector<std::function<double(int)>>,
      std::vector<EventFrame::column_t>>;
#else
  using col_accessor_type = std::vector<EventFrame::column_t>;
#endif

  col_accessor_type proj_cols;

  for (auto const &proj_col_name : projection_columns) {
    proj_cols.push_back(
        detail::eft::require_column_index<double>(ef, proj_col_name));
  }

  std::vector<double> projs(proj_cols.size(), 0);
  int num_rows = detail::eft::num_rows(ef);
  Eigen::ArrayXi binIds = Eigen::ArrayXi::Zero(num_rows);
  for (int row = 0; row < num_rows; ++row) {
    for (size_t pi = 0; pi < proj_cols.size(); ++pi) {
      projs[pi] = detail::eft::get_entry(ef, row, proj_cols[pi]);
    }
    binIds[row] = binning->find_bin(projs);
  }
  return binIds;
}

template <>
inline Eigen::ArrayXi find_bins(BinningPtr &binning,
                         std::shared_ptr<arrow::Table> const &ef,
                         std::vector<std::string> const &projection_columns) {

  Eigen::ArrayXi binIds = Eigen::ArrayXi::Zero(0);
  for (auto rb : arrow::TableBatchReader(ef)) {
    int nr = rb.ValueOrDie()->num_rows();
    if (!nr) {
      break;
    }
    Eigen::ArrayXi nbinIds = Eigen::ArrayXi::Zero(binIds.size() + nr);
    nbinIds.topRows(binIds.size()) = binIds;
    nbinIds.bottomRows(nr) =
        find_bins(binning, rb.ValueOrDie(), projection_columns);
    binIds = nbinIds;
  }
  return binIds;
}
} // namespace nuis