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

  py::class_<BinnedValuesBase::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &BinnedValuesBase::ColumnInfo::name)
      .def_readonly("column_label", &BinnedValuesBase::ColumnInfo::column_label)
      .def("__repr__", [](BinnedValuesBase::ColumnInfo const &s) {
        return fmt::format("Column: name={}{}", s.name,
                           s.column_label.size()
                               ? fmt::format(", label={}", s.column_label)
                               : std::string(""));
      });

  py::class_<BinnedValuesBase>(m, "BinnedValuesBase")
      .def_readonly_static("npos", &BinnedValuesBase::npos)
      .def_readonly("binning", &BinnedValuesBase::binning)
      .def_readonly("column_info", &BinnedValuesBase::column_info)
      .def("add_column", &BinnedValuesBase::add_column, py::arg("name"),
           py::arg("label") = "")
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &BinnedValuesBase::find_bin, py::const_))
      .def("find_bin",
           py::overload_cast<double>(&BinnedValuesBase::find_bin, py::const_));

  py::class_<HistFrame, BinnedValuesBase>(m, "HistFrame")
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readwrite("sumweights", &HistFrame::sumweights,
                     py::return_value_policy::reference_internal)
      .def_readwrite("variances", &HistFrame::variances,
                     py::return_value_policy::reference_internal)
      .def_readonly("num_fills", &HistFrame::num_fills)
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
      .def("finalise", &HistFrame::finalise,
           py::arg("divide_by_bin_sizes") = true)
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

  py::class_<BinnedValues, BinnedValuesBase>(m, "BinnedValues")
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readwrite("values", &BinnedValues::values,
                     py::return_value_policy::reference_internal)
      .def_readwrite("errors", &BinnedValues::errors,
                     py::return_value_policy::reference_internal)
      .def("make_HistFrame", &BinnedValues::make_HistFrame, py::arg("col") = 0)
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
