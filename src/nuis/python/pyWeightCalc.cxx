#include "nuis/frame/Frame.h"
#include "nuis/frame/FrameGen.h"

#include "nuis/weightcalc/IWeightCalc.h"
#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/weightcalc/plugins/plugins.h"

#include "nuis/python/pyNUISANCE.h"

#include "nuis/python/pyEventInput.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;
using namespace nuis;

// Only supporting simple for now
struct pyWeightCalc {

  pyWeightCalc(){};

  explicit pyWeightCalc(IWeightCalcHM3MapPtr p) { calc = p; }

  double calc_weight(HepMC3::GenEvent const &ev) {
    return calc->calc_weight(ev);
  }

  void set_parameters(std::map<std::string, double> const &par) {
    calc->set_parameters(par);
  }

  double operator()(HepMC3::GenEvent const &ev) { return calc_weight(ev); }

  IWeightCalcHM3MapPtr calc;
};

struct pyWeightCalcFactory {

  pyWeightCalcFactory() {}

  pyWeightCalc make(pyNormalizedEventSource &evs, YAML::Node const &cfg = {}) {
    auto calcs = wfact.make(evs.evs, cfg);
    return pyWeightCalc(calcs);
  }

  WeightCalcFactory wfact;
};

void pyWeightCalcInit(py::module &m) {

  py::class_<pyWeightCalc>(m, "WeightCalc")
      .def(py::init<>())
      .def("calc_weight", &pyWeightCalc::calc_weight)
      .def("set_parameters", &pyWeightCalc::set_parameters)
      .def("__call__", &pyWeightCalc::operator());

  py::class_<pyWeightCalcFactory>(m, "WeightCalcFactory")
      .def(py::init<>())
      .def("make", &pyWeightCalcFactory::make);

  // PS This doesn't actually build in my dev box please can we add a check?
  // Better yet could we recommend pluginss generate their own pybind
  // and we make a nuisance/pyNUISANCE/plugins/__init__.py to catch them?

  py::class_<Prob3plusplusWeightCalc, std::shared_ptr<Prob3plusplusWeightCalc>>(
      m, "Prob3plusplusWeightCalc")
      .def(py::init<YAML::Node const &>(), py::arg("config") = YAML::Node{})
      .def("calc_weight", &Prob3plusplusWeightCalc::calc_weight)
      .def("prob", &Prob3plusplusWeightCalc::prob)
      .def("set_parameters", &Prob3plusplusWeightCalc::set_parameters);
}
