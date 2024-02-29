#include "pybind11/pybind11.h"
#include "pybind11/stl_bind.h"

#include "yaml-cpp/yaml.h"

#include <iostream>

namespace py = pybind11;

// Python dictionary to YAML conversion
namespace PYBIND11_NAMESPACE {
namespace detail {
template <> struct type_caster<YAML::Node> {
public:
  PYBIND11_TYPE_CASTER(YAML::Node, const_name("node"));

  YAML::Node PyListToYAML(PyObject *pList) {
    YAML::Node n;

    for (Py_ssize_t i = 0; i < PyList_Size(pList); ++i) {

      PyObject *pVal = PyList_GetItem(pList, i);

      if (PyUnicode_Check(pVal)) {
        n[i] = std::string(PyUnicode_AsUTF8(pVal));

      } else if (PyLong_Check(pVal)) {
        n[i] = static_cast<int>(PyLong_AsLong(pVal));

      } else if (PyFloat_Check(pVal)) {
        n[i] = static_cast<double>(PyFloat_AsDouble(pVal));

      } else if (PyDict_Check(pVal)) {
        n[i] = PyDictToYAML(pVal);

      } else if (PyList_Check(pVal)) {
        n[i] = PyListToYAML(pVal);

      } else {
        std::cout << "Only Concrete Python instances allowed as "
                  << "dict inputs to YAML conversion!" << std::endl;

        std::cout << "Index [" << i << "] is problematic!" << std::endl;
      }
    }

    return n;
  }

  YAML::Node PyDictToYAML(PyObject *pDict) {
    YAML::Node n;

    PyObject *pKeys = PyDict_Keys(pDict);
    PyObject *pValues = PyDict_Values(pDict);

    for (Py_ssize_t i = 0; i < PyDict_Size(pDict); ++i) {
      PyObject *pKey = PyList_GetItem(pKeys, i);

      if (!PyUnicode_Check(pKey)) {
        std::cout << "YAML Conversion can only handle string keys!"
                  << std::endl;
      }

      std::string key = PyUnicode_AsUTF8(PyList_GetItem(pKeys, i));

      PyObject *pVal = PyList_GetItem(pValues, i);

      if (PyUnicode_Check(pVal)) {
        n[key] = std::string(PyUnicode_AsUTF8(pVal));

      } else if (PyLong_Check(pVal)) {
        n[key] = static_cast<int>(PyLong_AsLong(pVal));

      } else if (PyFloat_Check(pVal)) {
        n[key] = static_cast<double>(PyFloat_AsDouble(pVal));

      } else if (PyDict_Check(pVal)) {
        n[key] = PyDictToYAML(pVal);

      } else if (PyList_Check(pVal)) {
        n[key] = PyListToYAML(pVal);

      } else {
        std::cout << "Only Concrete Python instances allowed "
                  << " as dict inputs to YAML conversion!" << std::endl;

        std::cout << "KEY [" << key << "] is problematic!" << std::endl;
      }
    }

    return n;
  }

  bool load(handle src, bool) {
    /* Extract PyObject from handle */
    PyObject *source = src.ptr();

    if (PyUnicode_Check(source)) {
      value = std::string(PyUnicode_AsUTF8(source));

    } else if (PyLong_Check(source)) {
      value = static_cast<int>(PyLong_AsLong(source));

    } else if (PyFloat_Check(source)) {
      value = static_cast<double>(PyFloat_AsDouble(source));

    } else if (PyDict_Check(source)) {
      value = PyDictToYAML(source);

    } else if (PyList_Check(source)) {
      value = PyListToYAML(source);

    } else {
      std::cout << "Only Concrete Python instances allowed as "
                << "dict inputs to YAML conversion!" << std::endl;
    }

    return 1;
  }

  static handle cast(YAML::Node /*src*/, return_value_policy /* policy */,
                     handle /* parent */) {

    std::cout << "Cannot convert YAML to python yet! "
              << " See pyNUISANCE!" << std::endl;

    return PyLong_FromLong(0.0);
  }
};
} // namespace detail
} // namespace PYBIND11_NAMESPACE