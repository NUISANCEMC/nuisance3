#include "nuis/record/Comparison.h"
#include "nuis/record/IAnalysis.h"
#include "nuis/record/IRecord.h"
#include "nuis/record/RecordFactory.h"
#include "nuis/record/Utility.h"

#include "nuis/python/pyEventFrame.h"
#include "nuis/python/pyEventInput.h"
#include "nuis/python/pyNUISANCE.h"
#include "nuis/python/pyYAML.h"

#include "HepMC3/GenEvent.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#include "spdlog/fmt/bundled/core.h"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/python/pyarrow.h"
#endif

#include <string>

namespace py = pybind11;
using namespace nuis;

namespace nuis {

struct pyRecord {

  pyRecord() {}

  AnalysisPtr analysis(std::string name) { return rec->analysis(name); }
  auto get_analyses() { return rec->get_analyses(); }

  IRecordPtr rec;
};

struct pyRecordFactory {

  pyRecordFactory() {}

  pyRecord make(YAML::Node const &cfg = {}) {
    auto calcs = rfact.make(cfg);
    auto pyrec = pyRecord();
    pyrec.rec = calcs;
    return pyrec;
  }

  AnalysisPtr make_analysis(YAML::Node const &cfg = {}) {
    return rfact.make_analysis(cfg);
  }

  RecordFactory rfact;
};
} // namespace nuis

void pyRecordInit(py::module &m) {

  py::class_<Comparison, std::shared_ptr<Comparison>>(m, "Comparison")
      .def_readwrite("data", &Comparison::data)
      .def_readwrite("predictions", &Comparison::predictions)
      .def("likelihood", [](Comparison &comp) { return comp.likelihood(); });

  py::class_<IAnalysis::Target>(m, "IAnalysis_Target")
      .def_readonly("A", &IAnalysis::Target::A)
      .def_readonly("N", &IAnalysis::Target::N)
      .def_readonly("Z", &IAnalysis::Target::Z)
      .def_readonly("weight_by_mass", &IAnalysis::Target::weight_by_mass)
      .def("__str__", [](IAnalysis::Target const &tgt) {
        return fmt::format("{{ A = {}, Z = {}, weight_by_mass = {} }}", tgt.A,
                           tgt.Z, tgt.weight_by_mass);
      });

  py::class_<IAnalysis::XSScaling>(m, "IAnalysis_XSScaling")
      .def_readonly("units", &IAnalysis::XSScaling::units)
      .def_readonly("extra_scale_factor",
                    &IAnalysis::XSScaling::extra_scale_factor)
      .def_readonly("divide_by_bin_width",
                    &IAnalysis::XSScaling::divide_by_bin_width)
      .def("__str__", [](IAnalysis::XSScaling const &scl) {
        std::stringstream ss;
        ss << scl.units;
        return fmt::format("{{ units = {}, extra_scale_factor = {}, "
                           "divide_by_bin_width = {} }}",
                           ss.str(), scl.extra_scale_factor,
                           scl.divide_by_bin_width);
      });

  py::class_<IAnalysis, AnalysisPtr>(m, "IAnalysis")
      .def("process",
           [](AnalysisPtr const &ana, pyNormalizedEventSource &evs) {
             return ana->process(evs.evs);
           })
      .def("process",
           py::overload_cast<EventFrame const &>(&IAnalysis::process))
#ifdef NUIS_ARROW_ENABLED
      .def("process",
           [](AnalysisPtr const &ana, py::handle pyrb) {
             if (arrow::py::is_table(pyrb.ptr())) {
               return ana->process(
                   arrow::py::unwrap_table(pyrb.ptr()).ValueOrDie());
             } else if (arrow::py::is_batch(pyrb.ptr())) {
               return ana->process(
                   arrow::py::unwrap_batch(pyrb.ptr()).ValueOrDie());
             }
             throw std::runtime_error(
                 "IAnalysis::process passed an invalid py::handle");
           })
      .def("process",
           py::overload_cast<std::shared_ptr<arrow::RecordBatch> const &>(
               &IAnalysis::process))
#endif
      .def("add_to_framegen",
           [](AnalysisPtr const &ana, pyEventFrameGen &efg) {
             ana->add_to_framegen(*efg.gen);
           })
      .def("get_selection", &IAnalysis::get_selection)
      .def("get_projections", &IAnalysis::get_projections)
      .def("get_data", &IAnalysis::get_data)
      .def("get_probe_flux", &IAnalysis::get_probe_flux)
      .def("get_target", &IAnalysis::get_target)
      .def("get_covariance_matrix", &IAnalysis::get_covariance_matrix)
      .def("get_correlation_matrix", &IAnalysis::get_correlation_matrix)
      .def("get_smearing_matrix", &IAnalysis::get_smearing_matrix)
      .def("get_cross_section_scaling", &IAnalysis::get_cross_section_scaling)
      .def("prediction_generation_hint",
           &IAnalysis::prediction_generation_hint);

  py::class_<pyRecord>(m, "Record")
      .def(py::init<>())
      .def("analysis", &pyRecord::analysis)
      .def("get_analyses", &pyRecord::get_analyses)
      .def("__getattr__", &pyRecord::analysis);

  py::class_<pyRecordFactory>(m, "RecordFactory")
      .def(py::init<>())
      .def("make_record", &pyRecordFactory::make)
      .def("make_analysis", &pyRecordFactory::make_analysis);
}
