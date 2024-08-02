#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/fill_from_EventFrame.h"
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
}
