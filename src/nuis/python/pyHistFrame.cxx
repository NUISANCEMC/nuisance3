#include "nuis/HistFrame/Binning.h"
#include "nuis/HistFrame/BinningUtility.h"
#include "nuis/HistFrame/HistFrame.h"
#include "nuis/HistFrame/HistProjector.h"
#include "nuis/HistFrame/Utility.h"

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

  auto pyBinning = py::class_<Binning>(m, "Binning");

  py::class_<Binning::SingleExtent>(pyBinning, "SingleExtent")
      .def_readwrite("min", &Binning::SingleExtent::min)
      .def_readwrite("max", &Binning::SingleExtent::max)
      .def("width", &Binning::SingleExtent::width)
      .def("__repr__", &str_via_ss<Binning::SingleExtent>);

  pyBinning.def_readonly_static("npos", &Binning::npos)
      .def_readonly("bins", &Binning::bins)
      .def_readonly("axis_labels", &Binning::axis_labels)
      .def("bin_sizes", &Binning::bin_sizes)
      .def("__repr__", &str_via_ss<Binning>);

  pyBinning
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("find_bin",
           py::overload_cast<double>(&Binning::operator(), py::const_))
      .def("__call__", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("__call__",
           py::overload_cast<double>(&Binning::operator(), py::const_));

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
                  py::arg("label") = "")
      .def_static("from_extents", &Binning::from_extents, py::arg("extents"),
                  py::arg("labels") = std::vector<std::string>{})
      .def_static("product", &Binning::product, py::arg("ops"));

  py::class_<HistFrame::ColumnInfo>(m, "ColumnInfo")
      .def_readonly("name", &HistFrame::ColumnInfo::name,
                    py::return_value_policy::reference_internal)
      .def_readonly("dependent_axis_label",
                    &HistFrame::ColumnInfo::dependent_axis_label,
                    py::return_value_policy::reference_internal);

  pyBinning.def_static("get_bin_centers", &get_bin_centers)
      .def_static("get_bin_centers1D", &get_bin_centers1D);

  auto pyHistFrame =
      py::class_<HistFrame>(m, "HistFrame")
          .def(py::init<Binning, std::string const &, std::string const &>(),
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
          .def("fill_bin", &HistFrame::fill_bin, py::arg("bini"),
               py::arg("weight"), py::arg("col") = 0)
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
          .def("__repr__", &str_via_ss<HistFrame>);

  pyHistFrame
      .def_static(
          "project",
          py::overload_cast<HistFrame const &, std::vector<size_t> const &>(
              &Project))
      .def_static("project",
                  py::overload_cast<HistFrame const &, size_t>(&Project));

  py::module plotlymod = m.def_submodule("plotly", "plotly utilities");
  plotlymod.def("to_1D_json", &plotly::to_1D_json);

  py::module mplmod = m.def_submodule("matplotlib", "matplotlib utilities");
  mplmod.def("to_pcolormesh_data", &matplotlib::to_pcolormesh_data,
             py::arg("histframe"), py::arg("col") = 0);
}
