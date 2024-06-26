#pragma once

#include "nuis/binning/Binning.h"

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
} // namespace nuis