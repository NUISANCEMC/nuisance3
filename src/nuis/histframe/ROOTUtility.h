#pragma once

#include "nuis/histframe/BinningUtility.h"
#include "nuis/histframe/HistProjector.h"

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

// This should be header-only so that ROOT is not required by NUISANCE core
//   but is available for user scripts that want to write out ROOT histograms

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

std::vector<double> GetBinEdges(HistFrame const &hf, size_t dim) {
  auto projected_bins = project_to_unique_bins(hf.binning.bins, {
                                                                    dim,
                                                                });

  std::vector<double> contiguous_bin_edges = {
      projected_bins.front().front().min};
  for (auto bin : projected_bins) {
    auto dim_bin = bin.front();
    if (dim_bin.min != contiguous_bin_edges.back()) {
      spdlog::critical("bin edges are not contiguous: {} != {}", dim_bin.min,
                       contiguous_bin_edges.back());
      abort();
    }
    contiguous_bin_edges.push_back(dim_bin.max);
  }

  return contiguous_bin_edges;
}

std::unique_ptr<TH1> ToTH1(HistFrame const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           HistFrame::column_t col = 1) {
  auto hfp = Project(hf, 0);

  auto bins = GetBinEdges(hfp, 0);

  auto root_hist =
      std::make_unique<TH1D>(name.c_str(), "", bins.size() - 1, bins.data());

  Eigen::ArrayXd bin_scales = Eigen::ArrayXd::Constant(hfp.contents.rows(), 1);
  if (divide_by_bin_width) {
    bin_scales = hfp.binning.bin_sizes();
  }

  for (int bi = 0; bi < hfp.contents.rows(); ++bi) {
    root_hist->SetBinContent(bi + 1, hfp.contents(bi, col) / bin_scales(bi));
    root_hist->SetBinError(bi + 1,
                           std::sqrt(hfp.variance(bi, col) / bin_scales(bi)));
  }

  return root_hist;
}
std::unique_ptr<TH2> ToTH2(HistFrame const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           HistFrame::column_t col = 1) {
  auto hfp = Project(hf, {0, 1});

  auto binsx = GetBinEdges(hfp, 0);
  auto binsy = GetBinEdges(hfp, 1);

  auto root_hist =
      std::make_unique<TH2D>(name.c_str(), "", binsx.size() - 1, binsx.data(),
                             binsy.size() - 1, binsy.data());

  Eigen::ArrayXd bin_scales = Eigen::ArrayXd::Constant(hfp.contents.rows(), 1);
  if (divide_by_bin_width) {
    bin_scales = hfp.binning.bin_sizes();
  }

  for (size_t bix = 0; bix < binsx.size(); ++bix) {
    for (size_t biy = 0; biy < binsy.size(); ++biy) {

      Binning::BinExtents bin{{binsx[bix], binsy[biy]}};

      auto bi_it =
          std::find(hfp.binning.bins.begin(), hfp.binning.bins.end(), bin);

      if (bi_it == hfp.binning.bins.end()) {
        std::stringstream ss;
        ss << fmt::format(
                  "[ToTH2]: When looking for bin id [{},{}] = ({},{}), ({},{})",
                  bix, biy, binsx[bix], binsx[bix + 1], binsy[biy],
                  binsy[biy + 1])
           << ". Failed to find it.\nInput hist:" << hf
           << "\n  - Projected binning: " << hfp.binning.bins
           << "\n  - bin edges x = " << fmt::format("{}", binsx)
           << "\n  - bin edges y = " << fmt::format("{}", binsy) << std::endl;
        spdlog::critical(ss.str());
        abort();
      }

      auto bi = std::distance(hfp.binning.bins.begin(), bi_it);

      root_hist->SetBinContent(bix + 1, biy + 1,
                               hfp.contents(bi, col) / bin_scales(bi));
      root_hist->SetBinError(bix + 1, biy + 1,
                             std::sqrt(hfp.variance(bi, col) / bin_scales(bi)));
    }
  }
  return root_hist;
}
std::unique_ptr<TH3> ToTH3(HistFrame const &hf) { return nullptr; }
} // namespace nuis