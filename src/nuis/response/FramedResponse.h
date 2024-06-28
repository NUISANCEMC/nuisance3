#pragma once

#ifdef NUIS_ARROW_ENABLED
#include "nuis/eventframe/column_types.h"
#endif

#include "Eigen/Dense"

#include <array>

namespace Eigen {

template <size_t N, typename P = float> using ArrayNp = Eigen::Array<P, 1, N>;
template <typename P = float> using ArrayXXp = Array<P, Dynamic, Dynamic>;
template <typename P = float>
using ArrayXXpCRef = Ref<Eigen::Array<P, Dynamic, Dynamic> const, 0,
                         Stride<Eigen::Dynamic, Dynamic>> const;

} // namespace Eigen

namespace nuis {

template <size_t N, typename P = float> struct NaturalCubicFrameSpline {

  Eigen::ArrayNp<N, P> knot_x;
  Eigen::ArrayXXp<P> coeffs;

  NaturalCubicFrameSpline() {}

  template <typename iP> NaturalCubicFrameSpline(Eigen::ArrayNp<N, iP> x) {
    knot_x = x.template cast<iP>();
  }

  void build(Eigen::ArrayXXpCRef<P> yvals);

  template <typename iP>
  void build(
      std::enable_if_t<!std::is_same_v<P, iP>, Eigen::ArrayXXpCRef<iP>> yvals) {
    build(yvals.template cast<P>());
  }

  Eigen::Array<P, Eigen::Dynamic, 1> eval(P val);
};

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