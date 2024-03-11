#include "nuis/binning/Binning.h"
#include "nuis/binning/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

namespace py = pybind11;
using namespace nuis;

void pyBinningInit(py::module &m) {

  auto pyBinning = py::class_<Binning, std::shared_ptr<Binning>>(m, "Binning");

  py::class_<Binning::SingleExtent>(pyBinning, "SingleExtent")
      .def_readwrite("min", &Binning::SingleExtent::min)
      .def_readwrite("max", &Binning::SingleExtent::max)
      .def("width", &Binning::SingleExtent::width)
      .def("__repr__", &str_via_ss<Binning::SingleExtent>);

  pyBinning.def_readonly_static("npos", &Binning::npos)
      .def_readonly("bins", &Binning::bins)
      .def_readonly("axis_labels", &Binning::axis_labels)
      .def("bin_sizes", &Binning::bin_sizes)
      .def("__repr__", &str_via_ss<Binning>)
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("find_bin",
           py::overload_cast<double>(&Binning::operator(), py::const_))
      .def("__call__", py::overload_cast<std::vector<double> const &>(
                           &Binning::operator(), py::const_))
      .def("__call__",
           py::overload_cast<double>(&Binning::operator(), py::const_))
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
      .def_static("product", &Binning::product, py::arg("ops"))
      .def_static("get_bin_centers", &get_bin_centers)
      .def_static("get_bin_centers1D", &get_bin_centers1D)
      .def_static("log10_spaced_edges", &log10_spaced_edges)
      .def_static("ln_spaced_edges", &ln_spaced_edges)
      .def_static("uniform_width_edges", &uniform_width_edges)
      .def_static("lin_spaced_edges", &lin_spaced_edges)
      .def_static("cat_bin_edges", &cat_bin_edges)
      .def_static("edges_to_extents", &edges_to_extents);
}
