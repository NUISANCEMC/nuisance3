#pragma once

#include "nuis/histframe/HistFrame.h"

#include <vector>

namespace nuis {

DECLARE_NUISANCE_EXCEPT(EmptyData);
DECLARE_NUISANCE_EXCEPT(EmptyMC);

namespace finalise {

using func = std::function<BinnedValues(HistFrame const &, double const &)>;

inline func scale_to_cross_section(bool divide_by_bin_width) {
  return [=](HistFrame const &pred_hist, double const &fatx_per_sumw) {
    auto mc = pred_hist.finalise(divide_by_bin_width);
    mc.values *= fatx_per_sumw;
    mc.errors *= fatx_per_sumw;
    return mc;
  };
}

} // namespace finalise
} // namespace nuis
