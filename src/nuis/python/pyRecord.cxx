#include "nuis/record/Comparison.h"
#include "nuis/record/IRecord.h"
#include "nuis/record/RecordFactory.h"
#include "nuis/record/Table.h"
#include "nuis/record/Utility.h"

#include "nuis/python/pyNUISANCE.h"

#include "HepMC3/GenEvent.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#include <string>

namespace py = pybind11;
using namespace nuis;

namespace nuis {
using TablePtr = std::shared_ptr<Table>;
using IRecordPtr = std::shared_ptr<IRecord>;

struct pyRecord {

  pyRecord() {}

  // explicit pyRecord(YAML::Node const &cfg = {}) {}

  TablePtr table(std::string name) { return rec->table(name); }

  TablePtr operator[](std::string name) { return rec->table(name); }

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

void pyRecordInit(py::module &m) {

  // this is probably needed, but currently Juptyer notebooks crash hard if
  // ProSeleca::Get() has been called in this function.
  //  (void)nuis::database();

  py::class_<Comparison, std::shared_ptr<Comparison>>(m, "Comparison")
      .def_readwrite("mc", &Comparison::mc)
      .def_readwrite("data", &Comparison::data);

  py::class_<pyRecordFactory>(m, "RecordFactory")
      .def(py::init<>())
      .def("make", &pyRecordFactory::make);

  py::class_<pyRecord>(m, "Record")
      .def(py::init<>())
      .def("table", &pyRecord::table)
      .def("__getattr__", &pyRecord::table);

  py::class_<Table,TablePtr>(m, "Table")
      .def(py::init<>())
      .def_readwrite("blueprint", &Table::blueprint)
      .def_readwrite("clear", &Table::clear)
      .def_readwrite("select", &Table::select)
      .def_readwrite("project", &Table::project);
  //      .def_readwrite("likeihood", &Table::likeihood)
  //      .def("add_column", &Table::add_column)
  //      .def("find_column_index", &Table::find_column_index);
}
