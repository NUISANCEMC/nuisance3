#include "nuis/record/Comparison.h"
#include "nuis/record/IAnalysis.h"
#include "nuis/record/IRecord.h"
#include "nuis/record/RecordFactory.h"
#include "nuis/record/Utility.h"

#include "nuis/python/pyNUISANCE.h"
#include "nuis/python/pyYAML.h"

#include "HepMC3/GenEvent.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#include <string>

namespace py = pybind11;
using namespace nuis;

namespace nuis {

struct pyRecord {

  pyRecord() {}

  AnalysisPtr analysis(std::string name) { return rec->analysis(name); }

  AnalysisPtr operator[](std::string name) { return rec->analysis(name); }

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

  py::class_<IAnalysis, AnalysisPtr>(m, "IAnalysis")
      .def("process",
           py::overload_cast<INormalizedEventSourcePtr &>(&IAnalysis::process))
      .def("get_selection", &IAnalysis::get_selection)
      .def("get_projections", &IAnalysis::get_projections)
      .def("get_data", &IAnalysis::get_data)
      .def("prediction_generation_hint",
           &IAnalysis::prediction_generation_hint);

  py::class_<pyRecord>(m, "Record")
      .def(py::init<>())
      .def("analysis", &pyRecord::analysis)
      .def("__getattr__", &pyRecord::analysis);

  py::class_<pyRecordFactory>(m, "RecordFactory")
      .def(py::init<>())
      .def("make_record", &pyRecordFactory::make)
      .def("make_analysis", &pyRecordFactory::make_analysis);
}
