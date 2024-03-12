#include "nuis/convert/misc.h"
#include "nuis/convert/yaml.h"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyConvertInit(py::module &m) {
  m.def_submodule("convert", "")
      .def_submodule("HistFrame", "")
      .def("to_plotly1D", &to_plotly1D)
      .def("to_mpl_pcolormesh", &to_mpl_pcolormesh, py::arg("histframe"),
           py::arg("column") = 0)
      .def("to_yaml_str", &to_yaml_str)
      .def("from_yaml_str", &from_yaml_str);
}
