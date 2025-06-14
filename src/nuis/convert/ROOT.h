#pragma once

#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "nuis/histframe/BinnedValues.h"
#include "nuis/histframe/exceptions.h"
#include "nuis/histframe/utility.h"

#include "nuis/eventframe/missing_datum.h"

#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Types.hxx"

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

// This should be header-only so that ROOT is not required by NUISANCE core
//   but is available for user scripts that want to write out ROOT histograms

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace nuis {

template <typename BV>
std::unique_ptr<TH1> ToTH1(BV const &hf, std::string const &name,
                           bool divide_by_bin_width,
                           BinnedValuesBase::column_t col = 0) {

  if (col >= hf.column_info.size()) {
    throw InvalidColumnAccess()
        << "In ToTH1, tried to access column: " << col << ", but only "
        << hf.column_info.size() << " columns exist.";
  }

  auto hfp = Project(hf, 0);
  auto bins = get_bin_edges1D(hfp.binning->bins, 0);

  nuis_named_log("convert")::log_debug("[ToTH1]: bin edges: {}", bins);

  auto root_hist =
      std::make_unique<TH1D>(name.c_str(), "", bins.size() - 1, bins.data());

  Eigen::ArrayXd bin_scales =
      Eigen::ArrayXd::Constant(hfp.get_bin_contents().rows(), 1);
  if (divide_by_bin_width) {
    bin_scales = hfp.binning->bin_sizes();
  }

  nuis_named_log("convert")::log_debug("[ToTH1]: bin scales: {}", bin_scales);

  Eigen::ArrayXd contents = hfp.get_bin_contents().col(col) / bin_scales;
  Eigen::ArrayXd error = hfp.get_bin_uncertainty().col(col) / bin_scales;

  for (int bi = 0; bi < contents.rows(); ++bi) {
    root_hist->SetBinContent(bi + 1, contents(bi));
    root_hist->SetBinError(bi + 1, error(bi));

    nuis_named_log("convert")::log_debug("[ToTH1]: bin[{}] = {} +/- {}", bi + 1,
                                         contents(bi), error(bi));
  }

  root_hist->GetXaxis()->SetTitle(hf.binning->axis_labels.size()
                                      ? hf.binning->axis_labels.front().c_str()
                                      : "");
  root_hist->GetYaxis()->SetTitle(fmt::format("{} [{}]",
                                              hf.column_info[col].name,
                                              hf.column_info[col].column_label)
                                      .c_str());
  return root_hist;
}

template <typename BV>
std::unique_ptr<TH2> ToTH2(BV const &hf, std::string const &name,
                           bool divide_by_bin_area,
                           BinnedValuesBase::column_t col = 0) {

  if (col >= hf.column_info.size()) {
    throw InvalidColumnAccess()
        << "In ToTH1, tried to access column: " << col << ", but only "
        << hf.column_info.size() << " columns exist.";
  }

  auto hfp = Project(hf, std::vector<size_t>{0, 1});

  auto binsx = get_bin_edges1D(hfp.binning->bins, 0);
  auto binsy = get_bin_edges1D(hfp.binning->bins, 1);

  auto root_hist =
      std::make_unique<TH2D>(name.c_str(), "", binsx.size() - 1, binsx.data(),
                             binsy.size() - 1, binsy.data());

  Eigen::ArrayXd bin_scales =
      Eigen::ArrayXd::Constant(hfp.get_bin_contents().rows(), 1);
  if (divide_by_bin_area) {
    bin_scales = hfp.binning->bin_sizes();
  }

  Eigen::ArrayXd contents = hfp.get_bin_contents().col(col) / bin_scales;
  Eigen::ArrayXd error = hfp.get_bin_uncertainty().col(col) / bin_scales;

  for (size_t bix = 1; bix < binsx.size(); ++bix) {
    for (size_t biy = 1; biy < binsy.size(); ++biy) {

      auto bi = hfp.binning->find_bin({(binsx[bix - 1] + binsx[bix]) / 2.0,
                                       (binsy[biy - 1] + binsy[biy]) / 2.0});

      if (bi == Binning::npos) {
        std::stringstream ss;
        ss << fmt::format(
                  "[ToTH2]: When looking for bin id [{},{}] = ({},{}), ({},{})",
                  bix - 1, biy - 1, binsx[bix - 1], binsx[bix], binsy[biy - 1],
                  binsy[biy])
           << ". Failed to find it.\nInput hist:" << hf
           << "\n  - Projected binning: " << hfp.binning->bins
           << "\n  - bin edges x = " << fmt::format("{}", binsx)
           << "\n  - bin edges y = " << fmt::format("{}", binsy) << std::endl;
        log_critical(ss.str());
        throw CatastrophicBinningFailure();
      }

      root_hist->SetBinContent(bix, biy, contents(bi));
      root_hist->SetBinError(bix, biy, error(bi));
    }
  }
  return root_hist;
}

template <typename BV>
std::unique_ptr<TH3> ToTH3(BV const &hf, std::string const &name,
                           bool divide_by_bin_area,
                           BinnedValuesBase::column_t col = 0) {

  if (col >= hf.column_info.size()) {
    throw InvalidColumnAccess()
        << "In ToTH1, tried to access column: " << col << ", but only "
        << hf.column_info.size() << " columns exist.";
  }

  auto hfp = Project(hf, std::vector<size_t>{0, 1, 2});

  auto binsx = get_bin_edges1D(hfp.binning->bins, 0);
  auto binsy = get_bin_edges1D(hfp.binning->bins, 1);
  auto binsz = get_bin_edges1D(hfp.binning->bins, 2);

  auto root_hist = std::make_unique<TH3D>(
      name.c_str(), "", binsx.size() - 1, binsx.data(), binsy.size() - 1,
      binsy.data(), binsz.size() - 1, binsz.data());

  Eigen::ArrayXd bin_scales =
      Eigen::ArrayXd::Constant(hfp.get_bin_contents().rows(), 1);
  if (divide_by_bin_area) {
    bin_scales = hfp.binning->bin_sizes();
  }

  Eigen::ArrayXd contents = hfp.get_bin_contents().col(col) / bin_scales;
  Eigen::ArrayXd error = hfp.get_bin_uncertainty().col(col) / bin_scales;

  for (size_t bix = 1; bix < binsx.size(); ++bix) {
    for (size_t biy = 1; biy < binsy.size(); ++biy) {
      for (size_t biz = 1; biz < binsz.size(); ++biz) {

        auto bi = hfp.binning->find_bin({(binsx[bix - 1] + binsx[bix]) / 2.0,
                                         (binsy[biy - 1] + binsy[biy]) / 2.0,
                                         (binsz[biz - 1] + binsz[biz]) / 2.0});

        if (bi == Binning::npos) {
          std::stringstream ss;
          ss << fmt::format("[ToTH3]: When looking for bin id [{},{},{}] = "
                            "({},{}), ({},{}), ({},{})",
                            bix - 1, biy - 1, biz - 1, binsx[bix - 1],
                            binsx[bix], binsy[biy - 1], binsy[biy],
                            binsz[biz - 1], binsz[biz])
             << ". Failed to find it.\nInput hist:" << hf
             << "\n  - Projected binning: " << hfp.binning->bins
             << "\n  - bin edges x = " << fmt::format("{}", binsx)
             << "\n  - bin edges y = " << fmt::format("{}", binsy)
             << "\n  - bin edges z = " << fmt::format("{}", binsz) << std::endl;
          log_critical(ss.str());
          throw CatastrophicBinningFailure();
        }

        root_hist->SetBinContent(bix, biy, biz, contents(bi));
        root_hist->SetBinError(bix, biy, biz, error(bi));
      }
    }
  }
  
  return root_hist;
}

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

DECLARE_NUISANCE_EXCEPT(NonExistantInputROOTFile);
DECLARE_NUISANCE_EXCEPT(NonExistantTH1);

NuHepMC::GC4::EnergyDistribution get_EnergyDistribution_from_ROOT(
    std::string const &fname, std::string const &hname,
    std::string const &energy_unit = "", bool is_per_width = true) {
  NuHepMC::GC4::EnergyDistribution ed;
  ed.dist_type = NuHepMC::GC4::EDistType::kHistogram;
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
  ed.bin_edges = std::vector<double>(hist->GetXaxis()->GetNbins() + 1);
  ed.bin_edges[0] = hist->GetXaxis()->GetBinLowEdge(1);
  ed.bin_content = std::vector<double>(hist->GetXaxis()->GetNbins());
  for (int i = 0; i < hist->GetXaxis()->GetNbins(); ++i) {
    ed.bin_edges[i + 1] = hist->GetXaxis()->GetBinUpEdge(i + 1);
    ed.bin_content[i] = hist->GetBinContent(i + 1);
  }
  ed.ContentIsPerWidth = is_per_width;

  return ed;
}

} // namespace nuis
