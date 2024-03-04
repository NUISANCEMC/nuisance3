#pragma once

#include "nuis/record/ComparisonFrame.h"

namespace nuis {
namespace finalize {
// Fix these for full covariance estimates
void FATXNormalizedByBinWidth(ComparisonFrame& fr, const double fatx_by_pdf) {
    fr.mc.contents *= fatx_by_pdf;
    return;
}

void EventRateScaleToData(ComparisonFrame& fr, const double fatx_by_pdf) {

    double dt_sum = fr["data"].content.sum();
    if (dt_sum == 0.0) return;

    double mc_sum = fr["mc"].content.sum();
    if (mc_sum == 0.0) return;

    fr.mc.contents *= dt_sum / mc_sum;
    return;
}

}
}