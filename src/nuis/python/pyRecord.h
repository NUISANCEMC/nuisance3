#pragma once

#include <string> 

// #include "nuis/HistFrame/Binning.h"
// #include "nuis/HistFrame/HistFrame.h"
// #include "nuis/HistFrame/HistProjector.h"

#include "nuis/record/Table.h"
#include "nuis/record/IRecord.h"
#include "nuis/record/RecordFactory.h"
#include "nuis/record/ComparisonFrame.h"



#include "pybind11/eigen.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace nuis;

namespace nuis {
using TablePtr   = std::shared_ptr<Table>;
using IRecordPtr = std::shared_ptr<IRecord>;

struct pyRecord {

     pyRecord() {}

     // explicit pyRecord(YAML::Node const &cfg = {}) {}

     TablePtr table(std::string name) {
          return rec->table(name);
     }

     TablePtr operator[](std::string name) {
          return rec->table(name);
     }

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

  RecordFactory rfact;
};
} // namespace nuis



void init_record(py::module &m) {

     py::class_<ComparisonFrame>(m, "ComparisonFrame");
          // .def_readwrite("mc", ComparisonFrame::mc)
          // .def_readwrite("data", ComparisonFrame::data);

     py::class_<pyRecordFactory>(m, "RecordFactory")
          .def(py::init<>())
          .def("make", &pyRecordFactory::make);

     py::class_<pyRecord>(m, "Record")
          .def(py::init<>())
          .def("table", &pyRecord::table)
          .def("__getattr__", &pyRecord::table);

     py::class_<Table>(m, "Table")
          .def(py::init<>())
          .def_readwrite("blueprint", &Table::blueprint)
          .def_readwrite("clear", &Table::clear);
     //      .def_readwrite("select", &Table::select)
     //      .def_readwrite("project", &Table::finalize)
     //      .def_readwrite("likeihood", &Table::likeihood)
     //      .def("add_column", &Table::add_column)
     //      .def("find_column_index", &Table::find_column_index);







}
