#pragma once

#include "nuis/histframe/HistFrame.h"

#include <vector>

namespace nuis {

NEW_NUISANCE_EXCEPT(EmptyData);
NEW_NUISANCE_EXCEPT(EmptyMC);

namespace finalise {
// Fix these for full covariance estimates
BinnedValues FATXNormalizedByBinWidth(HistFrame const &pred_hist,
                                      const double fatx) {
  auto mc = pred_hist.finalise(true);
  mc.values /= fatx;
  mc.errors /= fatx;
  return mc;
}

BinnedValues FATXNormalized(HistFrame const &pred_hist, const double fatx) {
  auto mc = pred_hist.finalise(false);
  mc.values /= fatx;
  mc.errors /= fatx;
  return mc;
}

} // namespace finalise
} // namespace nuis
