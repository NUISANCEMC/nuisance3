#include "nuis/convert/misc.h"
#include "nuis/convert/yaml.h"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"

namespace py = pybind11;
using namespace nuis;

void pyConvertInit(py::module &m) {
  m.def_submodule("convert", "")
      .def_submodule("HistFrame", "")
      .def("to_plotly1D", py::overload_cast<HistFrame const &>(&to_plotly1D))
      .def("to_plotly1D", py::overload_cast<BinnedValues const &>(&to_plotly1D))
      .def("to_mpl_pcolormesh",
           py::overload_cast<HistFrame const &, HistFrame::column_t>(
               &to_mpl_pcolormesh),
           py::arg("histframe"), py::arg("column") = 0)
      .def("to_mpl_pcolormesh",
           py::overload_cast<BinnedValues const &, BinnedValues::column_t>(
               &to_mpl_pcolormesh),
           py::arg("histframe"), py::arg("column") = 0)
      .def("to_yaml_str", &to_yaml_str)
      .def("from_yaml_str", &from_yaml_str);
}
