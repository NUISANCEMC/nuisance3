#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

PYBIND11_MAKE_OPAQUE(std::vector<bool>);
PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<double>);
PYBIND11_MAKE_OPAQUE(std::vector<uint32_t>);

#include "nuis/python/pyYAML.h"

#include "nuis/python/pyEventInput.h"
#include "nuis/python/pyFrame.h"
#include "nuis/python/pyHistFrame.h"
#include "nuis/python/pyWeightCalc.h"
#include "nuis/python/pyRecord.h"

#include "yaml-cpp/yaml.h"

#include "spdlog/spdlog.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;

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

  // Check that the NUISANCEDB exists
  auto DATABASE = std::getenv("NUISANCEDB");
  if (!DATABASE) {
    spdlog::critical("NUISANCEDB environment variable is not set");
    abort();
  }

  m.add_object("hm", py::module::import("pyHepMC3"));
  // auto pps = py::module::import("pyProSelecta");
  // m.add_object("pps", pps);
  // m.add_object("pyProSelecta", pps);

  init_eventinput(m);
  init_frame(m);
  init_histframe(m);
  init_weightcalc(m);
  init_record(m);
  
}
