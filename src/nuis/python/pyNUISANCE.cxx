#include "nuis/python/pyNUISANCE.h"

#include "nuis/log.txx"

#include "nuis/python/pyEventInput.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;
using namespace nuis;

void pyEventFrameInit(py::module &);
void pyBinningInit(py::module &);
void pyHistFrameInit(py::module &);
#ifdef NUISANCE_USE_HEPDATA
void pyRecordInit(py::module &);
#endif
void pyWeightCalcInit(py::module &);
void pyNUISANCELogInit(py::module &);
void pyConvertInit(py::module &);
void pyResponseInit(py::module &);

PYBIND11_MODULE(_pyNUISANCE, m) {

  py::bind_vector<std::vector<bool>>(m, "Vector_bool");
  py::bind_vector<std::vector<int>>(m, "Vector_int");
  py::bind_vector<std::vector<double>>(m, "Vector_double");
  py::bind_vector<std::vector<uint32_t>>(m, "Vector_uint32_t");

  py::bind_vector<std::vector<std::vector<double>>>(m,
                                                    "Vector_Vector_double_t");

  py::implicitly_convertible<py::list, std::vector<bool>>();
  py::implicitly_convertible<py::list, std::vector<int>>();
  py::implicitly_convertible<py::list, std::vector<double>>();
  py::implicitly_convertible<py::list, std::vector<uint32_t>>();
  py::implicitly_convertible<py::list, std::vector<std::vector<double>>>();

  m.doc() = "NUISANCE implementation in python";

  m.add_object("hm", py::module::import("pyHepMC3"));
  m.add_object("nhm", py::module::import("pyNuHepMC"));
  m.add_object("pps", py::module::import("pyProSelecta"));

  pyEventInputInit(m);
  pyEventFrameInit(m);
  pyBinningInit(m);
  pyHistFrameInit(m);
#ifdef NUISANCE_USE_HEPDATA
  pyRecordInit(m);
#endif
  pyWeightCalcInit(m);
  pyNUISANCELogInit(m);
  pyConvertInit(m);
  pyResponseInit(m);
}
