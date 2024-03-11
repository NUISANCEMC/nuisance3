#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

namespace py = pybind11;
using namespace nuis;

std::map<std::string, Eigen::ArrayXd>
histframe_gettattr(HistFrame &s, std::string const &column) {
  auto cid = s.find_column_index(column);
  if (cid != HistFrame::npos) {
    return {{"contents", s.get_values(cid)}, {"error", s.get_errors(cid)}};
  }
  return {};
}

void pyHistFrameInit(py::module &m) {

  py::class_<HistFrame::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &HistFrame::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("dependent_axis_label",
                    &HistFrame::ColumnInfo::dependent_axis_label,
                    py::return_value_policy::reference_internal);

  py::class_<HistFrame>(m, "HistFrame")
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readonly_static("npos", &HistFrame::npos)
      .def_readonly_static("missing_datum", &HistFrame::missing_datum)
      .def_readonly("binning", &HistFrame::binning)
      .def_readwrite("contents", &HistFrame::contents,
                     py::return_value_policy::reference_internal)
      .def_readwrite("variance", &HistFrame::variance,
                     py::return_value_policy::reference_internal)
      .def_readonly("column_info", &HistFrame::column_info)
      .def_readonly("nfills", &HistFrame::nfills)
      .def("add_column", &HistFrame::add_column, py::arg("name"),
           py::arg("label") = "")
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &HistFrame::find_bin, py::const_))
      .def("find_bin",
           py::overload_cast<double>(&HistFrame::find_bin, py::const_))
      .def("fill_bin", &HistFrame::fill_bin, py::arg("bini"), py::arg("weight"),
           py::arg("col") = 0)
      .def("fill",
           py::overload_cast<std::vector<double> const &, double,
                             HistFrame::column_t>(&HistFrame::fill),
           py::arg("projections"), py::arg("weight"), py::arg("col") = 0)
      .def("fill",
           py::overload_cast<double, double, HistFrame::column_t>(
               &HistFrame::fill),
           py::arg("projection"), py::arg("weight"), py::arg("col") = 0)
      .def("fill_with_selection",
           py::overload_cast<int, std::vector<double> const &, double,
                             HistFrame::column_t>(
               &HistFrame::fill_with_selection),
           py::arg("selection"), py::arg("projections"), py::arg("weight"),
           py::arg("col") = 0)
      .def("fill_with_selection",
           py::overload_cast<int, double, double, HistFrame::column_t>(
               &HistFrame::fill_with_selection),
           py::arg("selection"), py::arg("projection"), py::arg("weight"),
           py::arg("col") = 0)
      .def("reset", &HistFrame::reset)
      // Pandas style data access
      .def("__getattr__", &histframe_gettattr)
      .def("__getitem__", &histframe_gettattr)
      .def("__repr__", &str_via_ss<HistFrame>)
      .def_static(
          "project",
          py::overload_cast<HistFrame const &, std::vector<size_t> const &>(
              &Project))
      .def_static("project",
                  py::overload_cast<HistFrame const &, size_t>(&Project));
}
