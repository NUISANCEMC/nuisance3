#pragma once

#include "nuis/record/Comparison.h"

namespace nuis {

NEW_NUISANCE_EXCEPT(EmptyData);
NEW_NUISANCE_EXCEPT(EmptyMC);

namespace finalize {
// Fix these for full covariance estimates
Comparison::Finalised FATXNormalizedByBinWidth(Comparison const &fr,
                                               const double fatx) {
  auto mc = fr.mc.finalise(true);
  mc.values /= fatx;
  mc.errors /= fatx;
  return Comparison::Finalised{fr.data, mc};
}

Comparison::Finalised EventRateScaleToData(Comparison const &fr,
                                           const double /*fatx_by_pdf*/) {

  auto datamc = fr[0];
  double dt_sum = datamc.data.value.sum();
  if (dt_sum == 0.0)
    throw EmptyData();

  // need to know here if we want to divide by fr.mc.binning->bin_sizes();
  double mc_sum = datamc.mc.count.sum();
  if (mc_sum == 0.0)
    throw EmptyMC();

  auto mc = fr.mc.finalise(true);
  mc.values *= dt_sum / mc_sum;
  mc.errors *= dt_sum / mc_sum;
  return Comparison::Finalised{fr.data, mc};
}

} // namespace finalize
} // namespace nuis