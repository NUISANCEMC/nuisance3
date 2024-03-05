#pragma once

#include "nuis/record/ComparisonFrame.h"

namespace nuis {
namespace likelihood {
// Fix these for full covariance estimates
double Chi2(const ComparisonFrame& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double Poisson(const ComparisonFrame& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double SimpleResidual(const ComparisonFrame& /*fr*/) {
    return 0.0; //return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}
}
}