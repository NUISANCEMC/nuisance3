
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

#include "yaml-cpp/yaml.h"

#include "nuis/measurement/Record.h"
#include "nuis/measurement/MeasurementLoader.h"
#include "nuis/measurement/HEPDataLoader.h"
#include "nuis/measurement/SimpleStatistics.h"

namespace py = pybind11;

// Python dictionary to YAML conversion
namespace PYBIND11_NAMESPACE { namespace detail {
template <> struct type_caster<YAML::Node> {
 public:

    PYBIND11_TYPE_CASTER(YAML::Node, const_name("node"));

    YAML::Node PyListToYAML(PyObject* pList) {
        YAML::Node n;

        for (Py_ssize_t i = 0; i < PyList_Size(pList); ++i) {

            PyObject* pVal = PyList_GetItem(pList,   i);

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
                std::cout << "Only Concrete Python instances allowed as " <<
                    "dict inputs to YAML conversion!" << std::endl;

                std::cout << "Index [" << i << "] is problematic!" << std::endl;
            }
        }

        return n;
    }

    YAML::Node PyDictToYAML(PyObject* pDict) {
        YAML::Node n;

        PyObject *pKeys = PyDict_Keys(pDict);
        PyObject *pValues = PyDict_Values(pDict);

        for (Py_ssize_t i = 0; i < PyDict_Size(pDict); ++i) {
            PyObject* pKey = PyList_GetItem(pKeys,   i);

            if (!PyUnicode_Check(pKey)) {
                std::cout <<
                    "YAML Conversion can only handle string keys!"
                    << std::endl;
            }

            std::string key = PyUnicode_AsUTF8(PyList_GetItem(pKeys, i));

            PyObject* pVal = PyList_GetItem(pValues,   i);

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
                std::cout << "Only Concrete Python instances allowed " <<
                    " as dict inputs to YAML conversion!" << std::endl;

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

    static handle cast(YAML::Node /*src*/,
        return_value_policy /* policy */,
        handle /* parent */) {

        std::cout << "Cannot convert YAML to python yet! " <<
            " See pyNUISANCE!" << std::endl;

        return PyLong_FromLong(0.0);
    }

};
}  // namespace detail
}  // namespace PYBIND11_NAMESPACE


namespace nuis{ 
namespace measurement {
// To avoid having to make trampoline classes for
// every possible inherited one we make a single
// measurement wrapper which contains in memory
// the base measurement object, calling a factory (eventually)
// to build. Python bindings all then assume that
// the they are dealing with a PyWrapper class.
class MeasurementPyWrapper {
public:
  MeasurementPyWrapper() {}

  explicit MeasurementPyWrapper(YAML::Node config) {
    meas = std::make_shared<HEPDataLoader>(HEPDataLoader(config));
  }

  std::vector<double> ProjectEvent(const HepMC3::GenEvent& event) {
    return meas->ProjectEvent(event);
  }

  bool FilterEvent(const HepMC3::GenEvent& event) {
    return meas->FilterEvent(event);
  }

  double WeightEvent(const HepMC3::GenEvent& event) {
    return meas->WeightEvent(event);
  }

  Record CreateRecord(const std::string label = "MC") {
    return meas->CreateRecord(label);
  }

  void FinalizeRecord(RecordPtr h, double scaling) {
    meas->FinalizeRecord(h, scaling);
  }

  std::shared_ptr<MeasurementLoader> meas;
};

}  // namespace measurement
}  // namespace nuis



void init_measurement(py::module &m) {
    
    m.doc() = "NUISANCE Measurement implementation in python";

    py::bind_vector<std::vector<std::vector<bool>>>\
        (m, "vector_vector_bool");

    py::bind_vector<std::vector<std::vector<int>>>\
        (m, "vector_vector_int");

    py::bind_vector<std::vector<std::vector<double>>>\
        (m, "vector_vector_double");

    py::bind_vector<std::vector<std::vector<uint32_t>>>\
        (m, "vector_vector_uint32_t");

    py::bind_map<std::map<std::string, double>>(m, "map_string_double");

    py::class_<nuis::measurement::Record>(m, "Record")
        .def(py::init<>())
        .def(py::init<YAML::Node>())
        .def("Reset",
            &nuis::measurement::Record::Reset)
        .def("GetBin",
            &nuis::measurement::Record::GetBin)
        .def("ResetBins",
            &nuis::measurement::Record::ResetBins)
        .def("GetMCCount",
            &nuis::measurement::Record::GetMCCounts)
        .def("GetMCWeight",
            &nuis::measurement::Record::GetMCWeight)
        .def("GetMCError",
            &nuis::measurement::Record::GetMCError)
        .def("GetMCError",
            &nuis::measurement::Record::GetMCError)
        .def("FillBinFromIndex",
            &nuis::measurement::Record::FillBinFromIndex)
        .def("FillBinFromProjection",
            &nuis::measurement::Record::FillBinFromProjection)
        .def_readwrite("label",
            &nuis::measurement::Record::label)
        .def_readwrite("name",
            &nuis::measurement::Record::name)
        .def_readwrite("title",
            &nuis::measurement::Record::title)
        .def_readwrite("bin_extent_low",
            &nuis::measurement::Record::bin_extent_low)
        .def_readwrite("bin_extent_high",
            &nuis::measurement::Record::bin_extent_high)
        .def_readwrite("bin_index",
            &nuis::measurement::Record::bin_index)
        .def_readwrite("bin_width",
            &nuis::measurement::Record::bin_width)
        .def_readwrite("bin_center",
            &nuis::measurement::Record::bin_center)
        .def_readwrite("bin_mask",
            &nuis::measurement::Record::bin_mask)
        .def_readwrite("data_value",
            &nuis::measurement::Record::data_value)
        .def_readwrite("data_covariance",
            &nuis::measurement::Record::data_covariance)
        .def_readwrite("data_error",
            &nuis::measurement::Record::data_error)
        .def_readwrite("mc_counts",
            &nuis::measurement::Record::mc_counts)
        .def_readwrite("mc_weights",
            &nuis::measurement::Record::mc_weights)
        .def_readwrite("mc_errors",
            &nuis::measurement::Record::mc_errors)
        .def_readwrite("total_mc_counts",
            &nuis::measurement::Record::total_mc_counts)
        .def_readwrite("total_mc_weights",
            &nuis::measurement::Record::total_mc_weights)
        .def_readwrite("total_mc_tally",
            &nuis::measurement::Record::total_mc_tally)
        .def("x",
            &nuis::measurement::Record::GetXCenter)
        .def("y",
            &nuis::measurement::Record::GetYCenter)
        .def("z",
            &nuis::measurement::Record::GetZCenter)
        .def("mc",
            &nuis::measurement::Record::GetMC)
        .def("xerr",
            &nuis::measurement::Record::GetXErr)
        .def("yerr",
            &nuis::measurement::Record::GetYErr)
        .def("zerr",
            &nuis::measurement::Record::GetZErr)
        .def("mcerr",
            &nuis::measurement::Record::GetMCErr);

    py::class_<nuis::measurement::Variables>(m, "variables")
        .def(py::init<>())
        .def_readwrite("index", &nuis::measurement::Variables::values)
        .def_readwrite("low", &nuis::measurement::Variables::low)
        .def_readwrite("high", &nuis::measurement::Variables::high)
        .def_readwrite("qualifiers", &nuis::measurement::Variables::qualifiers)
        .def_readwrite("valid", &nuis::measurement::Variables::valid)
        .def_readwrite("units", &nuis::measurement::Variables::units)
        .def_readwrite("name", &nuis::measurement::Variables::name)
        .def_readwrite("title", &nuis::measurement::Variables::title);

    py::class_<nuis::measurement::MeasurementPyWrapper>(m, "Measurement")
        .def(py::init<>())
        .def(py::init<YAML::Node>())
        .def("CreateRecord",
            &nuis::measurement::MeasurementPyWrapper::CreateRecord)
        .def("FinalizeRecord",
            &nuis::measurement::MeasurementPyWrapper::FinalizeRecord)
        .def("ProjectEvent",
            &nuis::measurement::MeasurementPyWrapper::ProjectEvent)
        .def("WeightEvent",
            &nuis::measurement::MeasurementPyWrapper::WeightEvent)
        .def("FilterEvent",
            &nuis::measurement::MeasurementPyWrapper::FilterEvent);

    m.def("CalculateRecordLikelihood",
            &nuis::measurement::CalculateRecordLikelihood);

}
