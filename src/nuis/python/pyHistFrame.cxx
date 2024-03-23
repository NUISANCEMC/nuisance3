#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyEventFrame.h"
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
           py::overload_cast<double>(&BinnedValuesBase::find_bin, py::const_))
      .def("find_column_index", &BinnedValuesBase::find_column_index);

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
      .def("project",
           [](HistFrame const &hf, std::vector<size_t> const &cols) {
             return Project(hf, cols);
           })
      .def("project",
           [](HistFrame const &hf, size_t col) { return Project(hf, col); })
      .def(
          "fill_from_EventFrame",
          [](HistFrame &hf, EventFrame &ef,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_from_EventFrame(hf, ef, projection_column_names,
                                        weight_column_names);
          },
          py::arg("eventframe"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
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
          "fill_from_RecordBatch",
          [](HistFrame &hf, std::shared_ptr<arrow::RecordBatch> &rb,
             std::vector<std::string> const &projection_column_names,
             std::vector<std::string> const &weight_column_names) {
            return fill_from_RecordBatch(hf, rb, projection_column_names,
                                         weight_column_names);
          },
          py::arg("recordbatch"), py::arg("projection_column_names"),
          py::arg("weight_column_names") =
              std::vector<std::string>{"weight.cv"})
#endif
      ;

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
      .def("project",
           [](BinnedValues const &bv, std::vector<size_t> const &cols) {
             return Project(bv, cols);
           })
      .def("project",
           [](BinnedValues const &bv, size_t col) { return Project(bv, col); });
}
