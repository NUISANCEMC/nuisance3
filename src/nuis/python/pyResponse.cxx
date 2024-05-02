#include "nuis/response/FramedResponse.h"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyResponseInit(py::module &m) {
  auto respmod = m.def_submodule("response", "");
  py::class_<NaturalCubicFrameSpline<4, double>>(respmod,
                                                 "NaturalCubicFrameSpline4")
      .def(py::init<Eigen::Array4d>())
      .def("build", &NaturalCubicFrameSpline<4, double>::build)
      .def("eval", &NaturalCubicFrameSpline<4, double>::eval)
      .def_readwrite("coeffs", &NaturalCubicFrameSpline<4, double>::coeffs);
}
