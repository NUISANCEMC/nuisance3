#include "nuis/binning/Binning.h"
#include "nuis/binning/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

namespace py = pybind11;
using namespace nuis;

void pyBinningInit(py::module &m) {

  auto pyBinning = py::class_<Binning, BinningPtr>(m, "Binning");

  py::class_<SingleExtent>(pyBinning, "SingleExtent")
      .def_readwrite("low", &SingleExtent::low)
      .def_readwrite("high", &SingleExtent::high)
      .def("width", &SingleExtent::width)
      .def("__str__", &str_via_ss<SingleExtent>);

  pyBinning.def_readonly_static("npos", &Binning::npos)
      .def_readonly("bins", &Binning::bins)
      .def_readonly("axis_labels", &Binning::axis_labels)
      .def("bin_sizes", &Binning::bin_sizes)
      .def("__str__", &str_via_ss<Binning>)
      .def("find_bin",
           [](BinningPtr binning, double x) { return binning->find_bin(x); })
      .def("find_bin",
           [](BinningPtr binning, std::vector<double> const &x) {
             return binning->find_bin(x);
           })
      .def_static("lin_space", &Binning::lin_space, py::arg("nbins"),
                  py::arg("start"), py::arg("stop"), py::arg("label") = "")
      .def_static("ln_space", &Binning::ln_space, py::arg("nbins"),
                  py::arg("start"), py::arg("stop"), py::arg("label") = "")
      .def_static("log10_space", &Binning::log10_space, py::arg("nbins"),
                  py::arg("start"), py::arg("stop"), py::arg("label") = "")
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
