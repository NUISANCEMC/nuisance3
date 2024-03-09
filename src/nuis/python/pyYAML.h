#pragma once

#include "nuis/python/pyNUISANCE.h"

#include "yaml-cpp/yaml.h"

#include <iostream>

// Python dictionary to YAML conversion
namespace PYBIND11_NAMESPACE {
namespace detail {
template <> struct type_caster<YAML::Node> {
public:
  PYBIND11_TYPE_CASTER(YAML::Node, const_name("node"));

  YAML::Node PyListToYAML(PyObject *pList);
  YAML::Node PyDictToYAML(PyObject *pDict);

  bool load(handle src, bool);

  static handle cast(YAML::Node /*src*/, return_value_policy /* policy */,
                     handle /* parent */);
};
} // namespace detail
} // namespace PYBIND11_NAMESPACE