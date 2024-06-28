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
}
