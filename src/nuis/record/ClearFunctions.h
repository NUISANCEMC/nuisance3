#pragma once
#include "nuis/record/ComparisonFrame.h"

namespace nuis {
namespace clear {
// Fix these for full covariance estimates
void DefaultClear(ComparisonFrame& fr) {
    fr.mc.reset();
    return;
}
}
}