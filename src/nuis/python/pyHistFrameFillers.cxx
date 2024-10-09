#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/fill_from_EventFrame.h"
#include "nuis/histframe/newfill.h"
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

  m.def(
       "fill_from_EventFrame",
       [](HistFrame &hf, EventFrame &ef,
          std::vector<std::string> const &projection_column_names,
          std::vector<std::string> const &weight_column_names) {
         return fill_from_EventFrame(hf, ef, projection_column_names,
                                     weight_column_names);
       },
       py::arg("eventframe"), py::arg("projection_column_names"),
       py::arg("weight_column_names") = std::vector<std::string>{"weight.cv"})
      .def(
          "fill_from_EventFrame_if",
          [](HistFrame &hf, EventFrame &ef,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_from_EventFrame_if(hf, ef, conditional_column_name,
                                           projection_column_names,
                                           weight_column_names);
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_new_column_from_EventFrame",
          [](HistFrame &hf, EventFrame &ef,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_name,
             std::vector<std::string> const &weight_column_names) {
            return fill_new_column_from_EventFrame(
                hf, ef, projection_column_names, column_name,
                weight_column_names);
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_new_column_from_EventFrame_if",
          [](HistFrame &hf, EventFrame &ef,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_name,
             std::vector<std::string> const &weight_column_names) {
            return fill_new_column_from_EventFrame_if(
                hf, ef, conditional_column_name, projection_column_names,
                column_name, weight_column_names);
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"), py::arg("column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_columns_from_EventFrame",
          [](HistFrame &hf, EventFrame &ef,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_selector_column_name,
             std::vector<std::string> const &weight_column_names) {
            return fill_columns_from_EventFrame(hf, ef, projection_column_names,
                                                column_selector_column_name,
                                                weight_column_names);
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_selector_column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_weighted_columns_from_EventFrame",
          [](HistFrame &hf, EventFrame &ef,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &column_weighter_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_weighted_columns_from_EventFrame(
                hf, ef, projection_column_names, column_weighter_names,
                weight_column_names);
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_weighter_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_columns_from_EventFrame_if",
          [](HistFrame &hf, EventFrame &ef,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_selector_column_name,
             std::vector<std::string> const &weight_column_names) {
            return fill_columns_from_EventFrame_if(
                hf, ef, conditional_column_name, projection_column_names,
                column_selector_column_name, weight_column_names);
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("column_selector_column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_procid_columns_from_EventFrame",
          [](HistFrame &hf, EventFrame &ef,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_procid_columns_from_EventFrame(
                hf, ef, projection_column_names, weight_column_names);
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_procid_columns_from_EventFrame_if",
          [](HistFrame &hf, EventFrame &ef,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_procid_columns_from_EventFrame_if(
                hf, ef, conditional_column_name, projection_column_names,
                weight_column_names);
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_from_EventFrameGen",
          [](HistFrame &hf, pyEventFrameGen &efg,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_from_EventFrameGen(
                hf, *efg.gen, projection_column_names, weight_column_names);
          },
          py::arg("eventframegen"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
#ifdef NUIS_ARROW_ENABLED
      .def(
          "fill_from_Arrow",
          [](HistFrame &hf, py::handle pyrb,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_from_Arrow(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_from_Arrow(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_from_Arrow_if",
          [](HistFrame &hf, py::handle pyrb,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_from_Arrow_if(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_from_Arrow_if(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_new_column_from_Arrow",
          [](HistFrame &hf, py::handle pyrb,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_name,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_new_column_from_Arrow(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_name, weight_column_names);

            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_new_column_from_Arrow(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_name, weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_new_column_from_Arrow_if",
          [](HistFrame &hf, py::handle pyrb,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_name,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_new_column_from_Arrow_if(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names, column_name,
                  weight_column_names);

            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_new_column_from_Arrow_if(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names, column_name,
                  weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"), py::arg("column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_columns_from_Arrow",
          [](HistFrame &hf, py::handle pyrb,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_selector_column_name,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_columns_from_Arrow(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_selector_column_name,
                  weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_columns_from_Arrow(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_selector_column_name,
                  weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_selector_column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_weighted_columns_from_Arrow",
          [](HistFrame &hf, py::handle pyrb,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &column_weighter_names,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_weighted_columns_from_Arrow(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_weighter_names,
                  weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_weighted_columns_from_Arrow(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, column_weighter_names,
                  weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("column_weighter_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_columns_from_Arrow_if",
          [](HistFrame &hf, py::handle pyrb,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::string const &column_selector_column_name,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_columns_from_Arrow_if(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  column_selector_column_name, weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_columns_from_Arrow_if(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  column_selector_column_name, weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("column_selector_column_name"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_procid_columns_from_Arrow",
          [](HistFrame &hf, py::handle pyrb,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_procid_columns_from_Arrow(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_procid_columns_from_Arrow(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  projection_column_names, weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
      .def(
          "fill_procid_columns_from_Arrow_if",
          [](HistFrame &hf, py::handle pyrb,
             std::string const &conditional_column_name,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            if (arrow::py::is_table(pyrb.ptr())) {
              return fill_procid_columns_from_Arrow_if(
                  hf, arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  weight_column_names);
            } else if (arrow::py::is_batch(pyrb.ptr())) {
              return fill_procid_columns_from_Arrow_if(
                  hf, arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie(),
                  conditional_column_name, projection_column_names,
                  weight_column_names);
            }
          },
          py::arg("eventframe"), py::arg("conditional_column_name"),
          py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
#endif
      ;

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
