#include "nuis/python/pyNUISANCE.h"

#include "nuis/log.txx"

#include "nuis/python/pyEventInput.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;
using namespace nuis;

void pyFrameInit(py::module &m);
void pyHistFrameInit(py::module &m);
void pyRecordInit(py::module &m);
void pyWeightCalcInit(py::module &m);
void pyNUISANCELog(py::module &m);

PYBIND11_MODULE(pyNUISANCE, m) {

  py::bind_vector<std::vector<bool>>(m, "Vector_bool");
  py::bind_vector<std::vector<int>>(m, "Vector_int");
  py::bind_vector<std::vector<double>>(m, "Vector_double");
  py::bind_vector<std::vector<uint32_t>>(m, "Vector_uint32_t");

  py::implicitly_convertible<py::list, std::vector<bool>>();
  py::implicitly_convertible<py::list, std::vector<int>>();
  py::implicitly_convertible<py::list, std::vector<double>>();
  py::implicitly_convertible<py::list, std::vector<uint32_t>>();

  m.doc() = "NUISANCE implementation in python";

  m.add_object("hm", py::module::import("pyHepMC3"));
  // auto pps = py::module::import("pyProSelecta");
  // m.add_object("pps", pps);
  // m.add_object("pyProSelecta", pps);

  pyEventInputInit(m);
  pyFrameInit(m);
  pyHistFrameInit(m);
  pyRecordInit(m);
  pyWeightCalcInit(m);
  pyNUISANCELog(m);
}
