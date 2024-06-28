#include "nuis/response/FramedResponse.h"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyResponseInit(py::module &m) {
  auto respmod = m.def_submodule("response", "");
  py::class_<NaturalCubicFrameSplineXd>(respmod, "NaturalCubicFrameSpline")
      .def(py::init<>())
      .def("build",
           [](NaturalCubicFrameSplineXd &self, Eigen::ArrayXd x,
              Eigen::ArrayXXCRef<double> y) {
             self.knot_x = x;
             self.build(y);
           })
      .def("eval", &NaturalCubicFrameSplineXd::eval)
      .def_readwrite("coeffs", &NaturalCubicFrameSplineXd::coeffs);

  py::class_<GaussRBFInterpolXd>(respmod, "GaussRBFInterpol")
      .def(py::init<>())
      .def("build",
           [](GaussRBFInterpolXd &self, Eigen::ArrayXXd x,
              Eigen::ArrayXXCRef<double> y) {
             self.knots = x;
             self.build(y);
           })
      .def("eval", &GaussRBFInterpolXd::eval)
      .def_readwrite("coeffs", &GaussRBFInterpolXd::coeffs);
}
