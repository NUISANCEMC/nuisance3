#pragma once

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

// This should be header-only so that ROOT is not required by NUISANCE core
//   but is available for user scripts that want to write out ROOT histograms

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

std::vector<double> GetBinEdges(nuis::HistFrame const &hf, size_t dim) {
  std::vector<Bins::SingleExtent> bin_extents;

  for (auto const &bin : hf.binning.bin_info.extents) {
    if (bin.size() <= dim) {
      spdlog::critical("Tried to get dimension {} extent from binning with "
                       "only {} dimensions.",
                       dim, bin.size());
      abort();
    }
    bin_extents.push_back(bin[dim]);
  }

  // size_t ctr = 0;
  // for (auto be : bin_extents) {
  //   spdlog::info("before{}:  {}-{}", ctr++, be.min, be.max);
  // }

  std::sort(bin_extents.begin(), bin_extents.end());
  bin_extents.erase(std::unique(bin_extents.begin(), bin_extents.end()),
                    bin_extents.end());

  // ctr = 0;
  // for (auto be : bin_extents) {
  //   spdlog::info("after{}:  {}-{}", ctr++, be.min, be.max);
  // }

  std::vector<double> contiguous_bin_edges = {bin_extents.front().min};
  for (auto be : bin_extents) {
    if (be.min != contiguous_bin_edges.back()) {
      spdlog::critical("bin edges are not contiguous: {} != {}", be.min,
                       contiguous_bin_edges.back());
      abort();
    }
    contiguous_bin_edges.push_back(be.max);
  }

  return contiguous_bin_edges;
}

std::unique_ptr<TH1> ToTH1(nuis::HistFrame const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           HistFrame::column_t col = 1) {
  auto bins = GetBinEdges(hf, 0);

  auto root_hist =
      std::make_unique<TH1D>(name.c_str(), "", bins.size() - 1, bins.data());

  Eigen::ArrayXd bin_scales = Eigen::ArrayXd::Constant(hf.contents.rows(),1);
  if (divide_by_bin_width) {
    bin_scales = hf.binning.bin_info.bin_sizes();
  }

  for (int bi = 0; bi < hf.contents.rows(); ++bi) {
    root_hist->SetBinContent(bi + 1, hf.contents(bi, col) / bin_scales(bi));
    root_hist->SetBinError(bi + 1,
                           std::sqrt(hf.variance(bi, col) / bin_scales(bi)));
  }

  return root_hist;
}
std::unique_ptr<TH2> ToTH2(nuis::HistFrame const &hf) { return nullptr; }
std::unique_ptr<TH3> ToTH3(nuis::HistFrame const &hf) { return nullptr; }
} // namespace nuis