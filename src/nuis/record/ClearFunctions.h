#pragma once
#include "nuis/record/Comparison.h"

namespace nuis {
namespace clear {
// Fix these for full covariance estimates
void DefaultClear(Comparison& fr) {
    fr.mc.reset();
    return;
}
}
}