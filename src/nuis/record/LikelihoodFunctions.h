#pragma once

#include "nuis/record/Comparison.h"

namespace nuis {
namespace likelihood {
// Fix these for full covariance estimates
double Chi2(const Comparison& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double Poisson(const Comparison& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double SimpleResidual(const Comparison& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}
}
}