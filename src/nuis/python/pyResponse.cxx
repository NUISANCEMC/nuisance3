#include "nuis/response/FramedResponse.h"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyResponseInit(py::module &m) {
  auto respmod = m.def_submodule("response", "");
  py::class_<NaturalCubicFrameSpline<4, double>>(respmod,
                                                 "NaturalCubicFrameSpline4")
      .def(py::init<>())
      .def("build",
           [](NaturalCubicFrameSpline<4, double> &self, Eigen::Array4d x,
              Eigen::ArrayXXpCRef<double> y) {
             self.knot_x = x;
             self.build(y);
           })
      .def("eval", &NaturalCubicFrameSpline<4, double>::eval)
      .def_readwrite("coeffs", &NaturalCubicFrameSpline<4, double>::coeffs);
}
