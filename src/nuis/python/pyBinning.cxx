#include "nuis/binning/Binning.h"
#include "nuis/binning/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/python/pyarrow.h"
#endif

namespace py = pybind11;
using namespace nuis;

void pyBinningInit(py::module &m) {

  auto pyBinning = py::class_<Binning, BinningPtr>(m, "Binning");

  py::class_<SingleExtent>(pyBinning, "SingleExtent")
      .def(py::init<double, double>())
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
      .def("find_bins",
           [](BinningPtr binning, EventFrame const &ef,
              std::vector<std::string> const &projection_columns) {
             return find_bins(binning, ef, projection_columns);
           })
      .def("find_bins",
           [](BinningPtr binning, EventFrame const &ef,
              std::string const &projection_column) {
             return find_bins(binning, ef,
                              {
                                  projection_column,
                              });
           })
#ifdef NUIS_ARROW_ENABLED
      .def(
          "find_bins",
          [](BinningPtr binning, py::handle pyarrobj,
             std::vector<std::string> const &projection_columns) {
            if (arrow::py::is_table(pyarrobj.ptr())) {
              return find_bins(
                  binning, arrow::py::unwrap_table(pyarrobj.ptr()).ValueOrDie(),
                  projection_columns);
            } else if (arrow::py::is_batch(pyarrobj.ptr())) {
              return find_bins(
                  binning, arrow::py::unwrap_batch(pyarrobj.ptr()).ValueOrDie(),
                  projection_columns);
            }
            throw std::runtime_error(
                "invalid type passed to find_bins, it must be a "
                "nuis::EventFrame or a pyarrow Table or RecordBatch.");
          })
      .def(
          "find_bins",
          [](BinningPtr binning, py::handle pyarrobj,
             std::string const &projection_column) {
            if (arrow::py::is_table(pyarrobj.ptr())) {
              return find_bins(
                  binning, arrow::py::unwrap_table(pyarrobj.ptr()).ValueOrDie(),
                  {
                      projection_column,
                  });
            } else if (arrow::py::is_batch(pyarrobj.ptr())) {
              return find_bins(
                  binning, arrow::py::unwrap_batch(pyarrobj.ptr()).ValueOrDie(),
                  {
                      projection_column,
                  });
            }
            throw std::runtime_error(
                "invalid type passed to find_bins, it must be a "
                "nuis::EventFrame or a pyarrow Table or RecordBatch.");
          })
#endif
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
      .def_static("get_bin_edges1D", &get_bin_edges1D, py::arg("bins"),
                  py::arg("ax") = 0)
      .def_static("get_bin_centers", &get_bin_centers)
      .def_static("get_bin_centers1D", &get_bin_centers1D)
      .def_static("log10_spaced_edges", &log10_spaced_edges)
      .def_static("ln_spaced_edges", &ln_spaced_edges)
      .def_static("uniform_width_edges", &uniform_width_edges)
      .def_static("lin_spaced_edges", &lin_spaced_edges)
      .def_static("cat_bin_edges", &cat_bin_edges)
      .def_static("edges_to_extents", &edges_to_extents)
      .def_static("get_sorted_bin_map", &get_sorted_bin_map)
      .def_static("get_rectilinear_grid", &get_rectilinear_grid);
}
