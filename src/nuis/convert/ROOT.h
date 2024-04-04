#pragma once

#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/eventframe/missing_datum.h"

#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Types.hxx"

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

#include "fmt/core.h"

// This should be header-only so that ROOT is not required by NUISANCE core
//   but is available for user scripts that want to write out ROOT histograms

NEW_NUISANCE_EXCEPT(NonContiguousBinning);

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

std::vector<double> GetBinEdges(BinnedValuesBase const &hf, size_t dim) {
  auto projected_bins = project_to_unique_bins(hf.binning->bins, {
                                                                     dim,
                                                                 });

  std::vector<double> contiguous_bin_edges = {
      projected_bins.front().front().low};
  for (auto bin : projected_bins) {
    auto dim_bin = bin.front();
    if (dim_bin.low != contiguous_bin_edges.back()) {
      log_critical("bin edges are not contiguous: {} != {}", dim_bin.low,
                   contiguous_bin_edges.back());
      throw NonContiguousBinning();
    }
    contiguous_bin_edges.push_back(dim_bin.high);
  }

  return contiguous_bin_edges;
}

template <typename BV>
std::unique_ptr<TH1> ToTH1(BV const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           BinnedValuesBase::column_t col = 1) {
  auto hfp = Project(hf, 0);
  auto bins = GetBinEdges(hfp, 0);

  auto root_hist =
      std::make_unique<TH1D>(name.c_str(), "", bins.size() - 1, bins.data());

  Eigen::ArrayXd bin_scales =
      Eigen::ArrayXd::Constant(hfp.get_bin_contents().rows(), 1);
  if (divide_by_bin_width) {
    bin_scales = hfp.binning->bin_sizes();
  }

  Eigen::ArrayXd contents = hfp.get_bin_contents().col(col) / bin_scales;
  Eigen::ArrayXd error = hfp.get_bin_uncertainty().col(col) / bin_scales;

  for (int bi = 0; bi < contents.rows(); ++bi) {
    root_hist->SetBinContent(bi + 1, contents(bi));
    root_hist->SetBinError(bi + 1, error(bi));
  }

  return root_hist;
}

template <typename BV>
std::unique_ptr<TH2> ToTH2(BV const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           BinnedValuesBase::column_t col = 1) {
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
        log_critical(ss.str());
        throw CatastrophicBinningFailure();
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

template <typename T> constexpr bool always_false_v = false;

template <typename TN> BinningPtr binning_from_ROOT(TN const &hist) {
  if constexpr (!std::is_same_v<TN, TH1> && !std::is_same_v<TN, TH2>) {
    static_assert(
        always_false_v<TN>,
        "binning_from_ROOT can only be templated over TH1 or TH2 currently.");
  }

  std::vector<double> binsx = {hist.GetXaxis()->GetBinLowEdge(1)};
  for (int i = 0; i < hist.GetXaxis()->GetNbins(); ++i) {
    binsx.push_back(hist.GetXaxis()->GetBinUpEdge(i + 1));
  }

  if constexpr (std::is_same_v<TN, TH1>) {
    return Binning::contiguous(binsx, hist.GetXaxis()->GetTitle());
  }
  if constexpr (std::is_same_v<TN, TH2>) {
    std::vector<double> binsy = {hist.GetYaxis()->GetBinLowEdge(1)};
    for (int i = 0; i < hist.GetYaxis()->GetNbins(); ++i) {
      binsy.push_back(hist.GetYaxis()->GetBinUpEdge(i + 1));
    }
    return Binning::product(
        {Binning::contiguous(binsx, hist.GetXaxis()->GetTitle()),
         Binning::contiguous(binsy, hist.GetYaxis()->GetTitle())});
  }
}

template <typename TN> BinnedValues BinnedValues_from_ROOT(TN const &hist) {
  if constexpr (!std::is_same_v<TN, TH1> && !std::is_same_v<TN, TH2>) {
    static_assert(
        always_false_v<TN>,
        "binning_from_ROOT can only be templated over TH1 or TH2 currently.");
  }

  BinnedValues bv(binning_from_ROOT<TN>(hist), hist.GetName());

  if constexpr (std::is_same_v<TN, TH1>) {
    for (int i = 0; i < hist.GetXaxis()->GetNbins(); ++i) {
      bv.values(i, 0) = hist.GetBinContent(i + 1);
      bv.errors(i, 0) = hist.GetBinError(i + 1);
    }
  }

  if constexpr (std::is_same_v<TN, TH2>) {
    size_t bi = 0;
    for (int j = 0; j < hist.GetYaxis()->GetNbins(); ++j) {
      for (int i = 0; i < hist.GetXaxis()->GetNbins(); ++i) {
        bv.values(bi, 0) = hist.GetBinContent(i + 1, j + 1);
        bv.errors(bi++, 0) = hist.GetBinError(i + 1, j + 1);
      }
    }
  }

  return bv;
}

NEW_NUISANCE_EXCEPT(NonExistantInputROOTFile);
NEW_NUISANCE_EXCEPT(NonExistantTH1);

NuHepMC::GC7::EnergyDistribution get_EnergyDistribution_from_ROOT(
    std::string const &fname, std::string const &hname,
    std::string const &energy_unit = "", bool is_per_width = true) {
  NuHepMC::GC7::EnergyDistribution ed;
  ed.dist_type = NuHepMC::GC7::EDistType::kHistogram;
  ed.MonoEnergeticEnergy = kMissingDatum<double>;

  TFile fin(fname.c_str());

  if (!fin.IsOpen() || fin.IsZombie()) {
    throw NonExistantInputROOTFile() << fname;
  }

  TH1 *hist = fin.Get<TH1>(hname.c_str());

  if (!hist) {
    throw NonExistantTH1() << hname << " does not exist in " << fname;
  }

  ed.energy_unit =
      energy_unit.size() ? energy_unit : hist->GetXaxis()->GetTitle();
  ed.rate_unit = hist->GetYaxis()->GetTitle();
  ed.bin_edges = Eigen::ArrayXd::Zero(hist->GetXaxis()->GetNbins() + 1);
  ed.bin_edges[0] = hist->GetXaxis()->GetBinLowEdge(1);
  ed.bin_content = Eigen::ArrayXd::Zero(hist->GetXaxis()->GetNbins());
  for (int i = 0; i < hist->GetXaxis()->GetNbins(); ++i) {
    ed.bin_edges[i + 1] = hist->GetXaxis()->GetBinUpEdge(i + 1);
    ed.bin_content[i] = hist->GetBinContent(i + 1);
  }
  ed.ContentIsPerWidth = is_per_width;

  return ed;
}

} // namespace nuis