#include "nuis/weightcalc/plugins/plugins.h"

#include "nuis/python/pyYAML.h"

#include "pybind11/pybind11.h"

namespace py = pybind11;
using namespace nuis;

PYBIND11_MODULE(pyNUISANCEProb3plusplus, m) {

  m.doc() = "NUISANCE Prob3plusplus WeightCalc interface";

  py::class_<Prob3plusplusWeightCalc, std::shared_ptr<Prob3plusplusWeightCalc>>(
      m, "Prob3plusplusWeightCalc")
      .def(py::init<YAML::Node const &>(), py::arg("config") = YAML::Node{})
      .def("calc_weight", &Prob3plusplusWeightCalc::calc_weight)
      .def("prob", &Prob3plusplusWeightCalc::prob)
      .def("set_parameters", &Prob3plusplusWeightCalc::set_parameters);
}
