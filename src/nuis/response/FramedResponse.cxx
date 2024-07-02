#include "nuis/response/FramedResponse.txx"

namespace nuis {

template class NaturalCubicFrameSpline<3, double>;
template class NaturalCubicFrameSpline<5, double>;
template class NaturalCubicFrameSpline<7, double>;
template class NaturalCubicFrameSpline<Eigen::Dynamic, double>;
template class NaturalCubicFrameSpline<3, float>;
template class NaturalCubicFrameSpline<5, float>;
template class NaturalCubicFrameSpline<7, float>;
template class NaturalCubicFrameSpline<Eigen::Dynamic, float>;

template class GaussRBFInterpol<double>;
// template class GaussRBFInterpol<float>;

} // namespace nuis