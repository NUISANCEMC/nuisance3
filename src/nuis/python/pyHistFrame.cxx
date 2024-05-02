#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/utility.h"

#include "nuis/log.txx"

#include "nuis/python/pyEventFrame.h"
#include "nuis/python/pyNUISANCE.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

namespace py = pybind11;
using namespace nuis;

void pyHistFrameFillersInit(py::class_<HistFrame, BinnedValuesBase> &);

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
      .def_readwrite("name", &BinnedValuesBase::ColumnInfo::name)
      .def_readwrite("column_label",
                     &BinnedValuesBase::ColumnInfo::column_label)
      .def("__str__", [](BinnedValuesBase::ColumnInfo const &s) {
        return fmt::format("Column: name={}{}", s.name,
                           s.column_label.size()
                               ? fmt::format(", label={}", s.column_label)
                               : std::string(""));
      });

  py::class_<BinnedValuesBase>(m, "BinnedValuesBase")
      .def_readonly_static("npos", &BinnedValuesBase::npos)
      .def_readonly("binning", &BinnedValuesBase::binning)
      .def_readwrite("column_info", &BinnedValuesBase::column_info)
      .def("add_column", &BinnedValuesBase::add_column, py::arg("name"),
           py::arg("label") = "")
      .def("find_bin", py::overload_cast<std::vector<double> const &>(
                           &BinnedValuesBase::find_bin, py::const_))
      .def("find_bin",
           py::overload_cast<double>(&BinnedValuesBase::find_bin, py::const_))
      .def("find_column_index", &BinnedValuesBase::find_column_index)
      .def("resize", &BinnedValuesBase::resize)
      .def("get_bin_contents", &BinnedValuesBase::get_bin_contents)
      .def("get_bin_uncertainty", &BinnedValuesBase::get_bin_uncertainty)
      .def("get_bin_uncertainty_squared",
           &BinnedValuesBase::get_bin_uncertainty_squared);

  py::class_<HistFrame, BinnedValuesBase> histframeclass(m, "HistFrame");
  histframeclass
      .def(py::init<BinningPtr, std::string const &, std::string const &>(),
           py::arg("binop"), py::arg("def_col_name") = "mc",
           py::arg("def_col_label") = "")
      .def_property(
          "sumweights",
          // Getter
          [](HistFrame &self) { return self.sumweights; },
          // Setter
          [](HistFrame &self, Eigen::ArrayXXdRef &val) {
            self.sumweights = val;
          })
      .def_property(
          "variances",
          // Getter
          [](HistFrame &self) { return self.variances; },
          // Setter
          [](HistFrame &self, Eigen::ArrayXXdRef &val) {
            self.variances = val;
          })
      .def_readonly("num_fills", &HistFrame::num_fills)
      .def("fill_bin", &HistFrame::fill_bin, py::arg("bini"), py::arg("weight"),
           py::arg("col"))
      .def("fill",
           py::overload_cast<std::vector<double> const &, double>(
               &HistFrame::fill),
           py::arg("projections"), py::arg("weight"))
      .def("fill", py::overload_cast<double, double>(&HistFrame::fill),
           py::arg("projection"), py::arg("weight"))
      .def("fill_if",
           py::overload_cast<bool, std::vector<double> const &, double>(
               &HistFrame::fill_if),
           py::arg("selected"), py::arg("projections"), py::arg("weight"))
      .def("fill_if",
           py::overload_cast<bool, double, double>(&HistFrame::fill_if),
           py::arg("selected"), py::arg("projection"), py::arg("weight"))
      .def("fill_column",
           py::overload_cast<std::vector<double> const &, double,
                             HistFrame::column_t>(&HistFrame::fill_column),
           py::arg("projections"), py::arg("weight"), py::arg("column"))
      .def("fill_column",
           py::overload_cast<double, double, HistFrame::column_t>(
               &HistFrame::fill_column),
           py::arg("projection"), py::arg("weight"), py::arg("column"))
      .def("fill_column_if",
           py::overload_cast<bool, std::vector<double> const &, double,
                             HistFrame::column_t>(&HistFrame::fill_column_if),
           py::arg("selected"), py::arg("projections"), py::arg("weight"),
           py::arg("column"))
      .def("fill_column_if",
           py::overload_cast<bool, double, double, HistFrame::column_t>(
               &HistFrame::fill_column_if),
           py::arg("selected"), py::arg("projection"), py::arg("weight"),
           py::arg("column"))
      .def("finalise", &HistFrame::finalise,
           py::arg("divide_by_bin_sizes") = true)
      .def("reset", &HistFrame::reset)
      .def("__getattr__", &histframe_gettattr)
      .def("__getitem__", &histframe_gettattr)
      .def("__copy__", [](HistFrame const &self) { return HistFrame(self); })
      .def("__str__", &str_via_ss<HistFrame>)
      .def("project",
           py::overload_cast<HistFrame const &, std::vector<size_t> const &,
                             bool>(&Project),
           py::arg("proj_to_axes"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<HistFrame const &, size_t, bool>(&Project),
           py::arg("proj_to_axis"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<HistFrame const &,
                             std::vector<std::string> const &, bool>(&Project),
           py::arg("proj_to_axes"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<HistFrame const &, std::string const &, bool>(
               &Project),
           py::arg("proj_to_axis"), py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<HistFrame const &, size_t, std::array<double, 2>,
                             bool, bool>(&Slice),
           py::arg("ax"), py::arg("slice_range"),
           py::arg("exclude_range_end_bin") = false,
           py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<HistFrame const &, size_t, double, bool>(&Slice),
           py::arg("ax"), py::arg("slice_val"),
           py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<HistFrame const &, std::string const &,
                             std::array<double, 2>, bool, bool>(&Slice),
           py::arg("ax"), py::arg("slice_range"),
           py::arg("exclude_range_end_bin") = false,
           py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<HistFrame const &, std::string const &, double,
                             bool>(&Slice),
           py::arg("ax"), py::arg("slice_val"),
           py::arg("result_has_binning") = true)
      .def("add", py::overload_cast<HistFrame const &, HistFrame const &>(&Add))
      .def("scale", py::overload_cast<HistFrame const &, double>(&Scale))
      .def("multiply",
           py::overload_cast<HistFrame const &, HistFrame const &>(&Multiply))
      .def("divide",
           py::overload_cast<HistFrame const &, HistFrame const &>(&Divide));

  pyHistFrameFillersInit(histframeclass);

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
      .def("__copy__",
           [](BinnedValues const &self) { return BinnedValues(self); })
      .def("__str__", &str_via_ss<BinnedValues>)
      .def("project",
           py::overload_cast<BinnedValues const &, std::vector<size_t> const &,
                             bool>(&Project),
           py::arg("proj_to_axes"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<BinnedValues const &, size_t, bool>(&Project),
           py::arg("proj_to_axis"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<BinnedValues const &,
                             std::vector<std::string> const &, bool>(&Project),
           py::arg("proj_to_axes"), py::arg("result_has_binning") = true)
      .def("project",
           py::overload_cast<BinnedValues const &, std::string const &, bool>(
               &Project),
           py::arg("proj_to_axis"), py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<BinnedValues const &, size_t,
                             std::array<double, 2>, bool, bool>(&Slice),
           py::arg("ax"), py::arg("slice_range"),
           py::arg("exclude_range_end_bin") = false,
           py::arg("result_has_binning") = true)
      .def(
          "slice",
          py::overload_cast<BinnedValues const &, size_t, double, bool>(&Slice),
          py::arg("ax"), py::arg("slice_val"),
          py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<BinnedValues const &, std::string const &,
                             std::array<double, 2>, bool, bool>(&Slice),
           py::arg("ax"), py::arg("slice_range"),
           py::arg("exclude_range_end_bin") = false,
           py::arg("result_has_binning") = true)
      .def("slice",
           py::overload_cast<BinnedValues const &, std::string const &, double,
                             bool>(&Slice),
           py::arg("ax"), py::arg("slice_val"),
           py::arg("result_has_binning") = true)
      .def("add",
           py::overload_cast<BinnedValues const &, BinnedValues const &>(&Add))
      .def("scale", py::overload_cast<BinnedValues const &, double>(&Scale))
      .def("multiply",
           py::overload_cast<BinnedValues const &, BinnedValues const &>(
               &Multiply))
      .def("divide",
           py::overload_cast<BinnedValues const &, BinnedValues const &>(
               &Divide));
}
