#pragma once

#include "nuis/except.h"

#include "Eigen/Dense"

#include "spdlog/spdlog.h"

#include <array>
#include <vector>

namespace Eigen {

template <size_t N, typename P = float> using ArrayNp = Eigen::Array<P, 1, N>;
template <typename P = float> using ArrayXXp = Array<P, Dynamic, Dynamic>;
template <typename P = float>
using ArrayXXpCRef = Ref<Eigen::Array<P, Dynamic, Dynamic> const, 0,
                         Stride<Eigen::Dynamic, Dynamic>> const;

} // namespace Eigen

namespace nuis {

NEW_NUISANCE_EXCEPT(EvalOutOfValidityRange);
NEW_NUISANCE_EXCEPT(InvalidYVals);

template <size_t N, typename P = double> struct NaturalCubicFrameSpline {

  Eigen::ArrayNp<N, P> x;
  Eigen::ArrayXXp<P> coeffs;

  NaturalCubicFrameSpline(Eigen::ArrayNp<N, P> xx) { x = xx; }

  // from
  // https://people.clas.ufl.edu/kees/files/CubicSplines.pdf
  void build(Eigen::ArrayXXpCRef<P> yvals) {

    if (yvals.cols() != x.size()) {
      throw InvalidYVals() << fmt::format(
          "NaturalCubicFrameSpline<{}> instantiated with {}  knots, but "
          "attemping to build spline with yvals matrix {} columns wide.",
          N, x.size(), yvals.cols());
    }

    coeffs = Eigen::ArrayXXpCRef<P>::Zero(yvals.rows(), 4 * (N - 1));

    Eigen::Array<double, 1, N - 1> h =
        x.template tail<N - 1>() - x.template head<N - 1>();
    Eigen::Matrix<double, N, N> A = Eigen::Array<double, N, N>::Zero();

    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        if (i == j) {

          if ((i == 0) || ((i + 1) == N)) {
            A(i, j) = 1;
          } else {
            A(i, j) = 2.0 * (h(i - 1) + h(i));
          }

        } else if (((i + 1) == j) && (i != 0)) { // upper diag
          A(i, j) = h(i);
        } else if (((i - 1) == j) && ((i + 1) != N)) { // lower diag
          A(i, j) = h(i - 1);
        }
      }
    }

    Eigen::ArrayNp<N - 1, P> b, d;
    Eigen::ArrayNp<N, P> c;

    Eigen::Array<double, N, 1> alpha = Eigen::Array<double, N, 1>::Zero();

    for (int row = 0; row < yvals.rows(); ++row) {

      // solve Ac = alpha for c, where alpha
      alpha.template segment<N - 2>(1) =
          (3.0 / h.template tail<N - 2>()) *
              (yvals.row(row).template tail<N - 2>() -
               yvals.row(row).template segment<N - 2>(1)) -
          (3.0 / h.template head<N - 2>()) *
              (yvals.row(row).template segment<N - 2>(1) -
               yvals.row(row).template head<N - 2>());

      // solve for c
      c = A.colPivHouseholderQr().solve(alpha.matrix()).array();

      d = (c.template tail<N - 1>() - c.template head<N - 1>()) / (3.0 * h);
      b = ((yvals.row(row).template tail<N - 1>() -
            yvals.row(row).template head<N - 1>()) /
           h) -
          ((h / 3.0) *
           (2.0 * c.template head<N - 1>() + c.template tail<N - 1>()));

      coeffs.row(row).template segment<N - 1>(0) =
          yvals.row(row).template head<N - 1>();
      coeffs.row(row).template segment<N - 1>(N - 1) = b;
      coeffs.row(row).template segment<N - 1>(2 * (N - 1)) =
          c.template head<N - 1>();
      coeffs.row(row).template segment<N - 1>(3 * (N - 1)) = d;
    }
  }

  Eigen::Array<P, Eigen::Dynamic, 1> eval(double val) {

    size_t i = 0;
    for (; i < (N - 1); ++i) {
      if ((x[i] <= val) && (val < x[i + 1])) {
        break;
      }
    }
    if (i == (N - 1)) {
      throw EvalOutOfValidityRange()
          << "eval passed " << val << ", but validity range: " << x[0] << " -- "
          << x[N - 1];
    }

    double past_knot = val - x[i];

    return coeffs.col(i) + past_knot * coeffs.col((N - 1) + i) +
           std::pow(past_knot, 2) * coeffs.col(2 * (N - 1) + i) +
           std::pow(past_knot, 3) * coeffs.col(3 * (N - 1) + i);
  }
};
} // namespace nuis