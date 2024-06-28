#pragma once

#ifdef NUIS_ARROW_ENABLED
#include "nuis/eventframe/column_types.h"
#endif

#include "Eigen/Dense"

#include <array>

namespace Eigen {

template <typename P = float> using RowArrayX = Array<P, 1, Dynamic>;
template <typename P = float> using ColArrayX = Array<P, Dynamic, 1>;
using RowArrayXd = RowArrayX<double>;
using ColArrayXd = ColArrayX<double>;
template <int N, typename P = float> using RowArray = Array<P, 1, N>;
template <int N, typename P = float> using ColArray = Array<P, N, 1>;
template <typename P = float> using ArrayXX = Array<P, Dynamic, Dynamic>;
template <typename P = float>
using ArrayXXCRef = Ref<Eigen::Array<P, Dynamic, Dynamic> const, 0,
                        Stride<Eigen::Dynamic, Dynamic>> const;

} // namespace Eigen

namespace nuis {

template <int N = Eigen::Dynamic, typename P = float>
struct NaturalCubicFrameSpline {

  Eigen::ColArray<N, P> knot_x;
  Eigen::ArrayXX<P> coeffs;

  NaturalCubicFrameSpline() {}

  template <typename iP> NaturalCubicFrameSpline(Eigen::ColArray<N, iP> x) {
    knot_x = x.template cast<P>();
  }

  void build(Eigen::ArrayXXCRef<P> yvals);

  template <typename iP>
  void build(
      std::enable_if_t<!std::is_same_v<P, iP>, Eigen::ArrayXXCRef<iP>> yvals) {
    build(yvals.template cast<P>());
  }

  Eigen::ColArrayX<P> eval(P val);
};

using NaturalCubicFrameSpline3d = NaturalCubicFrameSpline<3, double>;
using NaturalCubicFrameSpline5d = NaturalCubicFrameSpline<5, double>;
using NaturalCubicFrameSpline6d = NaturalCubicFrameSpline<6, double>;
using NaturalCubicFrameSplineXd =
    NaturalCubicFrameSpline<Eigen::Dynamic, double>;
using NaturalCubicFrameSpline3f = NaturalCubicFrameSpline<3, float>;
using NaturalCubicFrameSpline5f = NaturalCubicFrameSpline<5, float>;
using NaturalCubicFrameSpline6f = NaturalCubicFrameSpline<7, float>;
using NaturalCubicFrameSplineXf =
    NaturalCubicFrameSpline<Eigen::Dynamic, float>;

template <typename P = float> struct GaussRBFInterpol {

  int num_knots;
  Eigen::ArrayXX<P> knots;
  Eigen::ArrayXX<P> coeffs;
  P beta;

  GaussRBFInterpol() {}

  template <typename iP> GaussRBFInterpol(Eigen::ArrayXX<iP> x) {
    knots = x.template cast<P>();
  }

  void build(Eigen::ArrayXXCRef<P> yvals);

  template <typename iP>
  void build(
      std::enable_if_t<!std::is_same_v<P, iP>, Eigen::ArrayXXCRef<iP>> yvals) {
    build(yvals.template cast<P>());
  }

  Eigen::ColArrayX<P> eval(Eigen::RowArrayX<P> val);
};

using GaussRBFInterpolXd = GaussRBFInterpol<double>;

// #ifdef NUIS_ARROW_ENABLED

// template <typename P = float> struct NaturalCubicArrowSpline {

//   std::vector<std::string> col_names;
//   std::vector<P> knot_x;
//   std::shared_ptr<arrow::RecordBatch> coeffs;

//   NaturalCubicArrowSpline(std::vector<std::string> const &column_names,
//                           std::vector<P> const &x) {
//     col_names = column_names;
//     knot_x = x;
//   }

//   void build(std::shared_ptr<arrow::RecordBatch> yvals);

//   std::shared_ptr<typename column_type<From>::ATT::ArrayType> eval(P val);
// };

// #endif

} // namespace nuis