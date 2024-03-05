#include "nuis/python/pyYAML.h"

#include "nuis/python/pyEventInput.h"
#include "nuis/python/pyFrame.h"
#include "nuis/python/pyHistFrame.h"
#include "nuis/python/pyWeightCalc.h"

#include "yaml-cpp/yaml.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "spdlog/spdlog.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;

PYBIND11_MODULE(pyNUISANCE, m) {
  m.doc() = "NUISANCE implementation in python";

  // Check that the NUISANCEDB exists
  auto DATABASE = std::getenv("NUISANCEDB");
  if (!DATABASE) {
    spdlog::critical("NUISANCEDB environment variable is not set");
    abort();
  }

  m.add_object("hm", py::module::import("pyHepMC3"));
  auto pps = py::module::import("pyProSelecta");
  m.add_object("pps", pps);
  m.add_object("pyProSelecta", pps);

  init_eventinput(m);
  init_frame(m);
  init_histframe(m);
  init_weightcalc(m);
}
