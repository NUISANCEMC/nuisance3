#include "nuis/HistFrame/Binning.h"
#include "nuis/HistFrame/BinningUtility.h"
#include "nuis/HistFrame/HistFrame.h"
#include "nuis/HistFrame/HistProjector.h"
#include "nuis/HistFrame/Utility.h"

#include "nuis/python/pyYAML.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

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

void pyHistFrameInit(py::module &m) {

  auto pyBinning = py::class_<Binning>(m, "Binning");

  py::class_<Binning::SingleExtent>(pyBinning, "SingleExtent")
      .def_readwrite("min", &Binning::SingleExtent::min)
      .def_readwrite("max", &Binning::SingleExtent::max)
      .def("width", &Binning::SingleExtent::width)
      .def("__repr__", [](Binning::SingleExtent const &self) {
        std::stringstream ss;
        ss << self;
        return ss.str();
      });

  pyBinning.def_readonly_static("npos", &Binning::npos)
      .def_readonly("bins", &Binning::bins)
      .def_readonly("axis_labels", &Binning::axis_labels)
      .def("bin_sizes", &Binning::bin_sizes)
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("find_bin",
           py::overload_cast<double>(&Binning::operator(), py::const_))
      .def("__call__", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("__call__",
           py::overload_cast<double>(&Binning::operator(), py::const_))
      .def("__repr__", [](Binning const &self) {
        std::stringstream ss;
        ss << self;
        return ss.str();
      });

  pyBinning
      .def_static("lin_space", &Binning::lin_space, py::arg("nbins"),
                  py::arg("min"), py::arg("max"), py::arg("label") = "")
      .def_static("log_space", &Binning::log_space, py::arg("nbins"),
                  py::arg("min"), py::arg("max"), py::arg("label") = "")
      .def_static("log10_space", &Binning::log10_space, py::arg("nbins"),
                  py::arg("min"), py::arg("max"), py::arg("label") = "")
      .def_static("lin_spaceND", &Binning::lin_spaceND, py::arg("binnings"),
                  py::arg("labels") = std::vector<std::string>{})
      .def_static("contiguous", &Binning::contiguous, py::arg("binedges"),
                  py::arg("labels") = std::vector<std::string>{})
      .def_static("from_extents", &Binning::from_extents, py::arg("extents"),
                  py::arg("labels") = std::vector<std::string>{})
      .def_static("product", &Binning::product, py::arg("ops"));

  py::class_<HistFrame::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &HistFrame::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("dependent_axis_label",
                    &HistFrame::ColumnInfo::dependent_axis_label,
                    py::return_value_policy::reference_internal);

  py::class_<HistFrame>(m, "HistFrame")
      .def(py::init<Binning, std::string const &, std::string const &>(),
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

  pyBinning
      .def_static(
          "project",
          py::overload_cast<HistFrame const &, std::vector<size_t> const &>(
              &Project))
      .def_static("project",
                  py::overload_cast<HistFrame const &, size_t>(&Project))
      .def_static("get_bin_centers", &get_bin_centers)
      .def_static("get_bin_centers1D", &get_bin_centers1D);

  m.def("plotly1D", plotly1D);
  m.def("plotly2D", plotly2D);
}
