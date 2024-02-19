#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

#include "yaml-cpp/yaml.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

#include "spdlog/spdlog.h"

#include "nuis/frame/FrameGen.h"
#include "nuis/frame/Frame.h"

#include "nuis/weightcalc/IWeightCalc.h"
#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/python/pyEventInput.h"

namespace py = pybind11;


namespace nuis {

// Only supporting simple for now
struct pyWeightCalc {

  pyWeightCalc(){};

  explicit pyWeightCalc(IWeightCalcHM3MapPtr p){
    calc = p;
  }
  
  double CalcWeight(HepMC3::GenEvent const & ev) {
    return calc->CalcWeight(ev);
  }

  void SetParameters(std::map<std::string, double> const & par) {
    calc->SetParameters(par);
  }

  double operator()(HepMC3::GenEvent const & ev){
    return CalcWeight(ev);
  }

  IWeightCalcHM3MapPtr calc;
};

struct pyWeightCalcFactory {
  
  pyWeightCalcFactory() {}

  pyWeightCalc Make(pyNormalizedEventSource& evs, YAML::Node const &cfg = {}) {
    auto calcs = wfact.Make(evs.evs, cfg);
    return pyWeightCalc(calcs);
  }

  nuis::WeightCalcFactory wfact;
};
}

void init_weightcalc(py::module &m) {

  m.doc() = "NUISANCE implementation in python";

  py::class_<pyWeightCalc>(m, "WeightCalc")
      .def(py::init<>())
      .def("CalcWeight", &nuis::pyWeightCalc::CalcWeight)
      .def("SetParameters", &nuis::pyWeightCalc::SetParameters)
      .def("__call__", &nuis::pyWeightCalc::operator());


  py::class_<pyWeightCalcFactory>(m, "WeightCalcFactory")
      .def(py::init<>())
      .def("Make", &nuis::pyWeightCalcFactory::Make);

}

