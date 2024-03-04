#pragma once

#include "nuis/HistFrame/Binning.h"
#include "nuis/HistFrame/HistFrame.h"
#include "nuis/HistFrame/HistProjector.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace nuis;

std::map<std::string, Eigen::ArrayXd>
histframe_gettattr(HistFrame &s, std::string const &column) {
  auto cid = s.find_column_index(column);
  if (cid != HistFrame::npos) {
    return {{"contents", s.get_content(cid)}, {"error", s.get_error(cid)}};
  }
  return {};
}

void init_histframe(py::module &m) {

  auto pyBinningInfo = py::class_<Bins::BinningInfo>(m, "BinningInfo");

  py::class_<Bins::SingleExtent>(pyBinningInfo, "extent")
      .def_readwrite("min", &Bins::SingleExtent::min)
      .def_readwrite("max", &Bins::SingleExtent::max)
      .def("width", &Bins::SingleExtent::width)
      .def("__repr__", [](Bins::SingleExtent const &self) {
        std::stringstream ss;
        ss << self;
        return ss.str();
      });

  pyBinningInfo
      .def_readonly("extents", &Bins::BinningInfo::extents,
                    py::return_value_policy::reference_internal)
      .def_readonly("axis_labels", &Bins::BinningInfo::axis_labels,
                    py::return_value_policy::reference_internal)
      .def("bin_sizes", &Bins::BinningInfo::bin_sizes)
      .def("__repr__", [](Bins::BinningInfo const &self) {
        std::stringstream ss;
        ss << self;
        return ss.str();
      });

  py::class_<Bins::BinOp>(m, "BinOp")
      .def_readwrite("bin_info", &Bins::BinOp::bin_info)
      .def_readwrite("bin_func", &Bins::BinOp::bin_func);

  py::module binning = m.def_submodule("binning", "HistFrame binning bindings");
  binning.def("combine", &Bins::combine)
      .def("lin_space", &Bins::lin_space, py::arg("nbins"), py::arg("min"),
           py::arg("max"), py::arg("label") = "")
      .def("log_space", &Bins::log_space, py::arg("nbins"), py::arg("min"),
           py::arg("max"), py::arg("label") = "")
      .def("log10_space", &Bins::log10_space, py::arg("nbins"), py::arg("min"),
           py::arg("max"), py::arg("label") = "")
      .def("lin_spaceND", &Bins::lin_spaceND, py::arg("binnings"),
           py::arg("labels") = std::vector<std::string>{});

  py::class_<HistFrame::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &HistFrame::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("dependent_axis_label",
                    &HistFrame::ColumnInfo::dependent_axis_label,
                    py::return_value_policy::reference_internal);

  py::class_<HistFrame>(m, "HistFrame")
      .def(py::init<Bins::BinOp, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_readwrite("contents", &HistFrame::contents,
                     py::return_value_policy::reference_internal)
      .def_readwrite("variance", &HistFrame::variance,
                     py::return_value_policy::reference_internal)
      .def_readwrite("column_info", &HistFrame::column_info,
                     py::return_value_policy::reference_internal)
      .def_readwrite("nfills", &HistFrame::nfills,
                     py::return_value_policy::reference_internal)
      .def_readwrite("binning", &HistFrame::binning,
                     py::return_value_policy::reference_internal)
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
      .def("__str__", [](HistFrame const &s) {
        std::stringstream ss("");
        ss << HistFramePrinter(s);
        return ss.str();
      });

  m.def("Project", &Project);
}
