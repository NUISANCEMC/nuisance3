#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/frame_fill.h"
#include "nuis/histframe/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyEventFrame.h"
#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/python/pyarrow.h"
#endif

namespace py = pybind11;
using namespace nuis;

void pyHistFrameFillersInit(py::class_<HistFrame, BinnedValuesBase> &m) {

  py::class_<nuis::detail::FrameFillOp>(m, "FrameFillOp");

  m.def("fill_column", py::overload_cast<std::string const &>(&fill_column))
      .def("fill_column", py::overload_cast<int>(&fill_column))
      .def("fill_if", &fill_if)
      .def("weight_by", py::overload_cast<std::string const &>(&weight_by))
      .def("weight_by",
           py::overload_cast<std::vector<std::string> const &>(&weight_by))
      .def("weighted_column_map",
           py::overload_cast<std::string const &>(&weighted_column_map))
      .def("weighted_column_map",
           py::overload_cast<std::vector<std::string> const &>(
               &weighted_column_map))
      .def("categorize_by", &categorize_by)
      .def("split_by_ProcID", &split_by_ProcID)
      .def("no_CV_weight", &no_CV_weight);

  m.def(
       "fill",
       [](HistFrame &hf, EventFrame const &ef,
          std::vector<std::string> const &projection_column_names,
          std::vector<nuis::detail::FrameFillOp> const &operations) {
         detail::fill(hf, ef, projection_column_names, operations);
       },
       py::arg("EventFrame"), py::arg("projection_column_names"),
       py::arg("operations") = std::vector<nuis::detail::FrameFillOp>{})
      .def(
          "fill",
          [](HistFrame &hf, EventFrame const &ef,
             std::string const &projection_column_name,
             std::vector<nuis::detail::FrameFillOp> const &operations) {
            detail::fill(hf, ef,
                         {
                             projection_column_name,
                         },
                         operations);
          },
          py::arg("EventFrame"), py::arg("projection_column_name"),
          py::arg("operations") = std::vector<nuis::detail::FrameFillOp>{})
#ifdef NUIS_ARROW_ENABLED
      .def(
          "fill",
          [](HistFrame &hf, py::handle pyarrobj,
             std::vector<std::string> const &projection_column_names,
             std::vector<nuis::detail::FrameFillOp> const &operations) {
            if (arrow::py::is_table(pyarrobj.ptr())) {
              auto tab = arrow::py::unwrap_table(pyarrobj.ptr()).ValueOrDie();
              for (auto rb : arrow::TableBatchReader(tab)) {
                detail::fill(hf, rb.ValueOrDie(), projection_column_names,
                             operations);
              }
            } else if (arrow::py::is_batch(pyarrobj.ptr())) {
              auto rb = arrow::py::unwrap_batch(pyarrobj.ptr()).ValueOrDie();
              detail::fill(hf, rb, projection_column_names, operations);
            }
          },
          py::arg("EventFrame"), py::arg("projection_column_names"),
          py::arg("operations") = std::vector<nuis::detail::FrameFillOp>{})
      .def(
          "fill",
          [](HistFrame &hf, py::handle pyarrobj,
             std::string const &projection_column_name,
             std::vector<nuis::detail::FrameFillOp> const &operations) {
            if (arrow::py::is_table(pyarrobj.ptr())) {
              auto tab = arrow::py::unwrap_table(pyarrobj.ptr()).ValueOrDie();
              for (auto rb : arrow::TableBatchReader(tab)) {
                detail::fill(hf, rb.ValueOrDie(),
                             {
                                 projection_column_name,
                             },
                             operations);
              }
            } else if (arrow::py::is_batch(pyarrobj.ptr())) {
              auto rb = arrow::py::unwrap_batch(pyarrobj.ptr()).ValueOrDie();
              detail::fill(hf, rb,
                           {
                               projection_column_name,
                           },
                           operations);
            }
          },
          py::arg("EventFrame"), py::arg("projection_column_name"),
          py::arg("operations") = std::vector<nuis::detail::FrameFillOp>{})
#endif
      ;
}
