#pragma once

#include "nuis/record/Comparison.h"

namespace nuis {
namespace weight {
// Fix these for full covariance estimates
double DefaultWeight(HepMC3::GenEvent const &) {
    return 1;
}
}
}