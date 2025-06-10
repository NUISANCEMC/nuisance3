#include "nuis/weightcalc/plugins/nusystematicsWeightCalc.h"

#include "nuis/python/pyEventInput.h"
#include "nuis/python/pyYAML.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;
using namespace nuis;

PYBIND11_MODULE(pyNUISANCEnusystematics, m) {

  py::bind_vector<std::vector<uint32_t>>(m, "Vector_uint32_t");
  py::bind_vector<std::vector<double>>(m, "Vector_double");
  py::implicitly_convertible<py::list, std::vector<uint32_t>>();
  py::implicitly_convertible<py::list, std::vector<double>>();

  m.doc() = "NUISANCE nusystematics WeightCalc interface";

  py::class_<nusystematicsWeightCalc, std::shared_ptr<nusystematicsWeightCalc>>(
      m, "nusystematicsWeightCalc")
      .def(py::init([](pyNormalizedEventSource &pyevs, YAML::Node const &node) {
             return std::make_unique<nusystematicsWeightCalc>(
                 pyevs.evs->unwrap(), node);
           }),
           py::arg("evs"), py::arg("config") = YAML::Node{})
      .def("calc_weight", &nusystematicsWeightCalc::calc_weight)
      .def("calc_parameter_weights", &nusystematicsWeightCalc::calc_parameter_weights)
      .def("get_parameter_ids", &nusystematicsWeightCalc::get_parameter_ids)
      .def("set_parameters", &nusystematicsWeightCalc::set_parameters);
}
