
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

#include "yaml-cpp/yaml.h"

#include "nuis/measurement/Projection.h"
#include "nuis/measurement/IRecord.h"
#include "nuis/measurement/HEPDataRecord.h"
#include "nuis/measurement/Statistics.h"

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


    using ProjectionsFunc =
      std::function<std::vector<double>(HepMC3::GenEvent const &)>;
    
    using ProjectFunc =
        std::function<double(HepMC3::GenEvent const &)>;

    using FilterFunc =
        std::function<bool(HepMC3::GenEvent const &)>;

  MeasurementPyWrapper() {}

  explicit MeasurementPyWrapper(YAML::Node config) {
    meas = std::make_shared<HEPDataRecord>(HEPDataRecord(config));
    filter = meas->filter_func;
    projections = meas->proj_funcs;
  }

    // Currently this is needed for python->C++ propogation. Needs to be better.
  void Reconfigure(){
    meas->filter_func = filter;
    meas->proj_funcs = projections;
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

  Projection CreateProjection(const std::string label = "MC") {
    return meas->CreateProjection(label);
  }

  void FinalizeProjection(ProjectionPtr h, double scaling) {
    meas->FinalizeProjection(h, scaling);
  }

    void FillProjectionFromEvent(
      Projection& proj, const HepMC3::GenEvent& ev){
        meas->FillProjectionFromEvent(proj, ev);
      }



    // ProjectionsFunc GetProjectionFunction() {
    //     auto proj = meas->proj_funcs;

    //     return [proj](HepMC3::GenEvent const & ev) -> std::vector<double> {
    //         std::vector<double> vals;
    //         for (size_t i = 0; i < proj.size(); i++) {
    //             vals.emplace_back(proj[i](ev));
    //         }
    //         return vals;
    //     };
    // }

    // ProjectFunc GetProjectionFunction() {
    //     return meas->proj_funcs[0]; // Only working for 1D for now....
    // }

    // FilterFunc GetFilterFunction() {
    //     auto filter = meas->filter_func;

    //     return [filter](HepMC3::GenEvent const & ev) -> int {
    //         return filter(ev);
    //     };
    // }

    // This needs rethinking, we can edit filter and projections 
    // python side which is what we want, but need to figure
    // out a clean way to transmit between python->c++ side.
    // Can we just remove the original measurement?

    FilterFunc filter;
    std::vector<ProjectFunc> projections;
    std::shared_ptr<IRecord> meas;
};

}  // namespace measurement
}  // namespace nuis



void init_measurement(py::module &m) {
    
    m.doc() = "NUISANCE Measurement implementation in python";
    using ProjectFunc =
        std::function<double(HepMC3::GenEvent const &)>;

    py::bind_vector<std::vector<ProjectFunc>>(m, "Vector_project");

    py::bind_vector<std::vector<std::vector<bool>>>\
        (m, "vector_vector_bool");

    py::bind_vector<std::vector<std::vector<int>>>\
        (m, "vector_vector_int");

    py::bind_vector<std::vector<std::vector<double>>>\
        (m, "vector_vector_double");

    py::bind_vector<std::vector<std::vector<uint32_t>>>\
        (m, "vector_vector_uint32_t");

    // py::bind_map<std::map<int, Band>>(m, "map_band");

    py::bind_map<std::map<std::string, double>>(m, "map_string_double");

    py::class_<nuis::measurement::Band>(m, "Band")
        .def(py::init<>())
        .def(py::init<int, std::string, int>())
        .def_readwrite("label", &nuis::measurement::Band::label)
        .def_readwrite("band_id", &nuis::measurement::Band::band_id)
        .def_readwrite("value", &nuis::measurement::Band::value)
        .def_readwrite("count", &nuis::measurement::Band::count)
        .def_readwrite("error", &nuis::measurement::Band::error);

    // Don't love band as a name for each data or MC row...
    py::class_<nuis::measurement::Projection>(m, "Projection")
        .def(py::init<>())
        .def("Fill", &nuis::measurement::Projection::Fill)
        .def("Scale", &nuis::measurement::Projection::Scale)
        .def("Reset", &nuis::measurement::Projection::Reset)
        .def("AddBand", &nuis::measurement::Projection::AddBand)
        .def("CreateBand", &nuis::measurement::Projection::CreateBand)
        .def("__getitem__", &nuis::measurement::Projection::GetBandFromString)
        .def_readwrite("axis_label", &nuis::measurement::Projection::axis_label)
        .def_readwrite("bin_index", &nuis::measurement::Projection::bin_index)
        .def_readwrite("bin_mask", &nuis::measurement::Projection::bin_mask)
        .def_readwrite("bin_extent_low",
            &nuis::measurement::Projection::bin_extent_low)
        .def_readwrite("bin_extent_high",
            &nuis::measurement::Projection::bin_extent_high)
        .def("bin_center", &nuis::measurement::Projection::GetBinCenter )
        .def("bin_width", &nuis::measurement::Projection::GetBinWidth )
        .def("bin_halfwidth", &nuis::measurement::Projection::GetBinHalfWidth );

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
        .def("CreateProjection",
            &nuis::measurement::MeasurementPyWrapper::CreateProjection)
        .def("FillProjectionFromEvent",
            &nuis::measurement::MeasurementPyWrapper::FillProjectionFromEvent)
        .def("FinalizeProjection",
            &nuis::measurement::MeasurementPyWrapper::FinalizeProjection)
        .def("ProjectEvent",
            &nuis::measurement::MeasurementPyWrapper::ProjectEvent)
        .def("WeightEvent",
            &nuis::measurement::MeasurementPyWrapper::WeightEvent)
        .def("FilterEvent",
            &nuis::measurement::MeasurementPyWrapper::FilterEvent)
        .def("Reconfigure",
            &nuis::measurement::MeasurementPyWrapper::Reconfigure)
        .def_readwrite("projections",
            &nuis::measurement::MeasurementPyWrapper::projections)
        .def_readwrite("filter",
            &nuis::measurement::MeasurementPyWrapper::filter);

    m.def("CalculateProjectionLikelihood",
            &nuis::measurement::CalculateProjectionLikelihood);

}
