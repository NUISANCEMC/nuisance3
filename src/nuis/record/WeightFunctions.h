#pragma once

#include "nuis/record/ComparisonFrame.h"

namespace nuis {
namespace weight {
// Fix these for full covariance estimates
double DefaultWeight(HepMC3::GenEvent const &) {
    return 1;
}
}
}