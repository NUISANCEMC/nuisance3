#include "nuis/record/ComparisonFrame.h"

double CovarianceLikelihood(const ComparisonFrame& fr) {
    return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double PoissonLikelihood(const ComparisonFrame& fr) {
    return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double SimpleResidual(const ComparisonFrame& fr) {
    return (fr["data"].content - fr["mc"].content)/fr["data"].variance;
}

double CalculateLikelihood(const ComparisonFrame& fr) {

    double residual = 0.0;

    if (fr.likelihood_type == "covariance") {
        return CovarianceLikelihood(fr);
    } else if (fr.likelihood_type == "poisson") {
        return PoissonLikelihood(fr);
    } else {
        return SimpleResidual(fr)
    }

    return residual;
}
