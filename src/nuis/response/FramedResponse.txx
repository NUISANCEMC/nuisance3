#pragma once

#include "nuis/response/FramedResponse.h"

#include "nuis/except.h"

#include "fmt/core.h"

namespace nuis {

DECLARE_NUISANCE_EXCEPT(EvalOutOfValidityRange);
DECLARE_NUISANCE_EXCEPT(InvalidYVals);
DECLARE_NUISANCE_EXCEPT(InvalidNumParameters);

// from
// https://people.clas.ufl.edu/kees/files/CubicSplines.pdf
template <int N, typename P>
void NaturalCubicFrameSpline<N, P>::build(Eigen::ArrayXXCRef<P> yvals) {

  if (yvals.cols() != knot_x.size()) {
    throw InvalidYVals() << fmt::format(
        "NaturalCubicFrameSpline<{}> instantiated with {} knots, but "
        "attemping to build spline with yvals matrix {} columns wide.",
        N, knot_x.size(), yvals.cols());
  }

  if constexpr (N == Eigen::Dynamic) {
    int num_knots = knot_x.size();

    coeffs = Eigen::ArrayXX<P>::Zero(yvals.rows(), 4 * (num_knots - 1));

    Eigen::RowArrayXd h =
        (knot_x.tail(num_knots - 1) - knot_x.head(num_knots - 1))
            .template cast<double>();
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(num_knots, num_knots);

    for (int i = 0; i < num_knots; ++i) {
      for (int j = 0; j < num_knots; ++j) {
        if (i == j) {

          if ((i == 0) || ((i + 1) == num_knots)) {
            A(i, j) = 1;
          } else {
            A(i, j) = 2.0 * (h(i - 1) + h(i));
          }

        } else if (((i + 1) == j) && (i != 0)) { // upper diag
          A(i, j) = h(i);
        } else if (((i - 1) == j) && ((i + 1) != num_knots)) { // lower diag
          A(i, j) = h(i - 1);
        }
      }
    }

    Eigen::RowArrayXd b, d;
    Eigen::RowArrayXd c;

    Eigen::ColArrayXd alpha = Eigen::RowArrayXd::Zero(num_knots);

    for (int row = 0; row < yvals.rows(); ++row) {

      // solve Ac = alpha for c, where alpha
      alpha.segment(1, num_knots - 2) =
          (3.0 / h.tail(num_knots - 2)) *
              (yvals.row(row).tail(num_knots - 2) -
               yvals.row(row).segment(1, num_knots - 2))
                  .template cast<double>() -
          (3.0 / h.head(num_knots - 2)) *
              (yvals.row(row).segment(1, num_knots - 2) -
               yvals.row(row).head(num_knots - 2))
                  .template cast<double>();

      // solve for c
      c = A.colPivHouseholderQr().solve(alpha.matrix()).array();

      d = (c.tail(num_knots - 1) - c.head(num_knots - 1)) / (3.0 * h);
      b = ((yvals.row(row).tail(num_knots - 1) -
            yvals.row(row).head(num_knots - 1))
               .template cast<double>() /
           h) -
          ((h / 3.0) * (2.0 * c.head(num_knots - 1) + c.tail(num_knots - 1)));

      coeffs.row(row).segment(0, num_knots - 1) =
          yvals.row(row).head(num_knots - 1).template cast<P>();
      coeffs.row(row).segment(num_knots - 1, num_knots - 1) = b.cast<P>();
      coeffs.row(row).segment(2 * (num_knots - 1), num_knots - 1) =
          c.head(num_knots - 1).cast<P>();
      coeffs.row(row).segment(3 * (num_knots - 1), num_knots - 1) = d.cast<P>();
    }

  } else {

    coeffs = Eigen::ArrayXX<P>::Zero(yvals.rows(), 4 * (N - 1));

    Eigen::RowArray<N - 1, double> h =
        (knot_x.template tail<N - 1>() - knot_x.template head<N - 1>())
            .template cast<double>();
    Eigen::Matrix<double, N, N> A = Eigen::Array<double, N, N>::Zero();

    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
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

    Eigen::RowArray<N - 1, double> b, d;
    Eigen::RowArray<N, double> c;

    Eigen::ColArray<N, double> alpha = Eigen::RowArray<N, double>::Zero();

    for (int row = 0; row < yvals.rows(); ++row) {

      // solve Ac = alpha for c, where alpha
      alpha.template segment<N - 2>(1) =
          (3.0 / h.template tail<N - 2>()) *
              (yvals.row(row).template tail<N - 2>() -
               yvals.row(row).template segment<N - 2>(1))
                  .template cast<double>() -
          (3.0 / h.template head<N - 2>()) *
              (yvals.row(row).template segment<N - 2>(1) -
               yvals.row(row).template head<N - 2>())
                  .template cast<double>();

      // solve for c
      c = A.colPivHouseholderQr().solve(alpha.matrix()).array();

      d = (c.template tail<N - 1>() - c.template head<N - 1>()) / (3.0 * h);
      b = ((yvals.row(row).template tail<N - 1>() -
            yvals.row(row).template head<N - 1>())
               .template cast<double>() /
           h) -
          ((h / 3.0) *
           (2.0 * c.template head<N - 1>() + c.template tail<N - 1>()));

      coeffs.row(row).template segment<N - 1>(0) =
          yvals.row(row).template head<N - 1>().template cast<P>();
      coeffs.row(row).template segment<N - 1>(N - 1) = b.template cast<P>();
      coeffs.row(row).template segment<N - 1>(2 * (N - 1)) =
          c.template head<N - 1>().template cast<P>();
      coeffs.row(row).template segment<N - 1>(3 * (N - 1)) =
          d.template cast<P>();
    }
  }
}

template <int N, typename P>
Eigen::ColArrayX<P> NaturalCubicFrameSpline<N, P>::eval(P val) {

  int num_knots = N;
  if constexpr (N == Eigen::Dynamic) {
    num_knots = knot_x.size();
  }

  int i = 0;
  for (; i < (num_knots - 1); ++i) {
    if ((knot_x[i] <= val) && (val < knot_x[i + 1])) {
      break;
    }
  }
  if ((i == (num_knots - 1)) && (val != knot_x[num_knots - 1])) {
    throw EvalOutOfValidityRange()
        << "eval passed " << val << ", but validity range: " << knot_x[0]
        << " -- " << knot_x[num_knots - 1];
  }

  P past_knot = val - knot_x[i];

  return coeffs.col(i) + past_knot * coeffs.col((num_knots - 1) + i) +
         std::pow(past_knot, 2) * coeffs.col(2 * (num_knots - 1) + i) +
         std::pow(past_knot, 3) * coeffs.col(3 * (num_knots - 1) + i);
}

// from
// https://en.wikipedia.org/wiki/Radial_basis_function_network#Interpolation
template <typename P>
void GaussRBFInterpol<P>::build(Eigen::ArrayXXCRef<P> yvals) {

  if (yvals.cols() != knots.rows()) {
    throw InvalidYVals() << fmt::format(
        "GaussRBFInterpol instantiated with {} knots, but "
        "attemping to build spline with yvals matrix {} columns wide.",
        knots.rows(), yvals.cols());
  }

  num_knots = knots.rows();

  coeffs = Eigen::ArrayXX<P>::Zero(yvals.rows(), num_knots);
  Eigen::ArrayXXd r2 = Eigen::ArrayXXd::Zero(num_knots, num_knots);

  for (int i = 0; i < num_knots; ++i) {
    for (int j = 0; j < num_knots; ++j) {
      r2(i, j) = (knots.row(i) - knots.row(j)).square().sum();
    }
  }

  beta = -std::sqrt(r2.maxCoeff()) * 0.5;

  Eigen::MatrixXd gij = (beta * r2).exp();

  for (int row = 0; row < yvals.rows(); ++row) {
    coeffs.row(row) = gij.colPivHouseholderQr()
                          .solve(yvals.row(row).transpose().matrix())
                          .array()
                          .template cast<P>();
  }
}

template <typename P>
Eigen::ColArrayX<P> GaussRBFInterpol<P>::eval(Eigen::RowArrayX<P> vals) {

  if (vals.size() != knots.cols()) {
    throw InvalidNumParameters() << fmt::format(
        "GaussRBFInterpol instantiated with knots of {} dims, but "
        "attemping to evaluate response with {} parameters.",
        knots.cols(), vals.size());
  }

  Eigen::RowArrayX<P> g =
      (beta * ((knots.rowwise() - vals).square()).rowwise().sum()).exp();
  return (coeffs.rowwise() * g).rowwise().sum();
}

} // namespace nuis
