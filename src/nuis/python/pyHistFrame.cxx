#include "nuis/frame/missing_datum.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

namespace py = pybind11;
using namespace nuis;

std::map<std::string, Eigen::ArrayXdRef>
histframe_gettattr(HistFrame &s, std::string const &column) {
  auto cid = s.find_column_index(column);
  if (cid != HistFrame::npos) {
    auto [count, variance] = s[cid];
    return {{"count", count}, {"variance", variance}};
  }
  return {};
}

std::map<std::string, Eigen::ArrayXdRef>
binnedvalues_gettattr(BinnedValues &s, std::string const &column) {
  auto cid = s.find_column_index(column);
  if (cid != BinnedValues::npos) {
    auto [value, error] = s[cid];
    return {{"value", value}, {"error", error}};
  }
  return {};
}

void pyHistFrameInit(py::module &m) {

  py::class_<HistFrame::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &HistFrame::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("column_label",
                    &HistFrame::ColumnInfo::column_label,
                    py::return_value_policy::reference_internal);

  py::class_<HistFrame>(m, "HistFrame")
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readonly_static("npos", &HistFrame::npos)
      .def_readonly_static("missing_datum", &kMissingDatum)
      .def_readonly("binning", &HistFrame::binning)
      .def_readwrite("sumweights", &HistFrame::sumweights,
                     py::return_value_policy::reference_internal)
      .def_readwrite("variances", &HistFrame::variances,
                     py::return_value_policy::reference_internal)
      .def_readonly("column_info", &HistFrame::column_info)
      .def_readonly("num_fills", &HistFrame::num_fills)
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
      .def("__getattr__", &histframe_gettattr)
      .def("__getitem__", &histframe_gettattr)
      .def("__repr__", &str_via_ss<HistFrame>)
      .def_static(
          "project",
          py::overload_cast<HistFrame const &, std::vector<size_t> const &>(
              &Project))
      .def_static("project",
                  py::overload_cast<HistFrame const &, size_t>(&Project));

  py::class_<BinnedValues::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &BinnedValues::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("column_label",
                    &BinnedValues::ColumnInfo::column_label,
                    py::return_value_policy::reference_internal);

  py::class_<BinnedValues>(m, "BinnedValues")
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readonly_static("npos", &BinnedValues::npos)
      .def_readonly_static("missing_datum", &kMissingDatum)
      .def_readonly("binning", &BinnedValues::binning)
      .def_readwrite("values", &BinnedValues::values,
                     py::return_value_policy::reference_internal)
      .def_readwrite("errors", &BinnedValues::errors,
                     py::return_value_policy::reference_internal)
      .def_readonly("column_info", &BinnedValues::column_info)
      .def("add_column", &BinnedValues::add_column, py::arg("name"),
           py::arg("label") = "")
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &BinnedValues::find_bin, py::const_))
      .def("find_bin",
           py::overload_cast<double>(&BinnedValues::find_bin, py::const_))
      .def("__getattr__", &binnedvalues_gettattr)
      .def("__getitem__", &binnedvalues_gettattr)
      .def("__repr__", &str_via_ss<BinnedValues>)
      .def_static(
          "project",
          py::overload_cast<BinnedValues const &, std::vector<size_t> const &>(
              &Project))
      .def_static("project",
                  py::overload_cast<BinnedValues const &, size_t>(&Project));
}
