
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, double>);

#include "yaml-cpp/yaml.h"

//#include "HepMC3/Reader.h"
//#include "HepMC3/ReaderFactory.h"
//#include "HepMC3/GenEvent.h"

//#include "ProSelecta/ProSelecta.h"

#include "nuis/measurement/Record.h"
#include "nuis/measurement/MeasurementLoader.h"
#include "nuis/measurement/HEPDataLoader.h"
#include "nuis/measurement/SimpleStatistics.h"

std::vector<std::string> files_to_read;
std::string filename;

std::string analysis_symname;
std::string sel_symname;
std::vector<std::string> projection_symnames;
std::vector<std::string> wgt_symnames;

std::vector<std::string> include_paths;

std::string ProSelecta_env_dir;

namespace py = pybind11;

void configure_environment(){
    // for (auto const &p : include_paths) {
        // ProSelecta::Get().AddIncludePath(p);
    // }

    char *ProSelecta_env = getenv("PROSELECTA_DIR");
    if (!ProSelecta_env && !ProSelecta_env_dir.length()) {
        std::cout << "[ERROR]: Cannot find ProSelecta environment headers. Either "
                    "define PROSELECTA_DIR in the calling environment or add "
                    "--env command line argument."
                << std::endl;
        return;
    }
    if (!ProSelecta_env_dir.length()) {
        ProSelecta_env_dir = ProSelecta_env;
        if (ProSelecta_env_dir.back() != '/') {
        ProSelecta_env_dir += '/';
        }
        ProSelecta_env_dir += "ProSelecta/env/";
    }

    ProSelecta::Get().AddIncludePath(ProSelecta_env_dir);


    std::string hepmc3dir = std::string(getenv("NUHEPMC3_INC"));
    ProSelecta::Get().AddIncludePath(hepmc3dir);


    bool read_env = ProSelecta::Get().LoadText("#include \"env.h\"",
                                                ProSelecta::Interpreter::kCling);

    if (!read_env) {
        std::cout
            << "[ERROR]: Cling failed to interpret the processor environment, if "
            "you passed the right path to find these header files and this "
            "still occures then it is a bug in ProSelectaCPP itself."
            << std::endl;
        abort();
    }
}

// Python dictionary to YAML conversion
namespace PYBIND11_NAMESPACE { namespace detail {
    template <> struct type_caster<YAML::Node> {
    public:
        PYBIND11_TYPE_CASTER(YAML::Node, const_name("node"));
        
        YAML::Node PyListToYAML(PyObject* pList){
            YAML::Node n;

            for (Py_ssize_t i = 0; i < PyList_Size(pList); ++i) {
                
                PyObject* pVal = PyList_GetItem(pList,   i);
                if (PyUnicode_Check(pVal)) n[i] = std::string( PyUnicode_AsUTF8(pVal) );
                else if (PyLong_Check(pVal)) n[i] = int( PyLong_AsLong(pVal) );
                else if (PyFloat_Check(pVal)) n[i] = double( PyFloat_AsDouble(pVal) );
                else if (PyDict_Check(pVal)) n[i] = PyDictToYAML(pVal);
                else if (PyList_Check(pVal)) n[i] = PyListToYAML(pVal);
                else {
                    std::cout << "Only Concrete Python instances allowed as dict inputs to YAML conversion!" << std::endl;
                    std::cout << "Index [" << i << "] is problematic!" << std::endl;
                }
            }

            return n;
        }
        YAML::Node PyDictToYAML(PyObject* pDict){

            YAML::Node n;

            PyObject *pKeys = PyDict_Keys(pDict);
            PyObject *pValues = PyDict_Values(pDict);
            for (Py_ssize_t i = 0; i < PyDict_Size(pDict); ++i) {
                
                PyObject* pKey = PyList_GetItem(pKeys,   i);
                if (!PyUnicode_Check(pKey)){
                    std::cout << "YAML Conversion can only handle string keys!" << std::endl;
                }
                std::string key = PyUnicode_AsUTF8( PyList_GetItem(pKeys,   i));

                PyObject* pVal = PyList_GetItem(pValues,   i);
                if (PyUnicode_Check(pVal)) n[key] = std::string( PyUnicode_AsUTF8(pVal) );
                else if (PyLong_Check(pVal)) n[key] = int( PyLong_AsLong(pVal) );
                else if (PyFloat_Check(pVal)) n[key] = double( PyFloat_AsDouble(pVal) );
                else if (PyDict_Check(pVal)) n[key] = PyDictToYAML(pVal);
                else if (PyList_Check(pVal)) n[key] = PyListToYAML(pVal);
                else {
                    std::cout << "Only Concrete Python instances allowed as dict inputs to YAML conversion!" << std::endl;
                    std::cout << "KEY [" << key << "] is problematic!" << std::endl;
                }
            }

            return n;
        }

        bool load(handle src, bool) {
            /* Extract PyObject from handle */
            PyObject *source = src.ptr();

            if (PyUnicode_Check(source)) value = std::string( PyUnicode_AsUTF8(source) );
            else if (PyLong_Check(source)) value = int( PyLong_AsLong(source) );
            else if (PyFloat_Check(source)) value = double( PyFloat_AsDouble(source) );
            else if (PyDict_Check(source)) value = PyDictToYAML(source);
            else if (PyList_Check(source)) value = PyListToYAML(source);
            else {
                std::cout << "Only Concrete Python instances allowed as dict inputs to YAML conversion!" << std::endl;
            }

            return 1;
        }

        static handle cast(YAML::Node src, return_value_policy /* policy */, handle /* parent */) {
            std::cout << "Cannot convert YAML to python yet! See pyNUISANCE!" << std::endl;
            return PyLong_FromLong(0.0);
        }
    };
}} // namespace PYBIND11_NAMESPACE::detail


PYBIND11_MODULE(pyMEASUREMENT, m) {
    m.doc() = "NUISANCE implementation in python";

    py::bind_vector<std::vector<bool>>(m, "VectorBOOL");
    py::bind_vector<std::vector<int>>(m, "VectorINT");
    py::bind_vector<std::vector<double>>(m, "VectorDOUBLE");
    py::bind_vector<std::vector<uint32_t>>(m, "VectorUINT32");

    py::bind_vector<std::vector<std::vector<bool>>>(m, "VectorVectorBOOL");
    py::bind_vector<std::vector<std::vector<int>>>(m, "VectorVectorINT");
    py::bind_vector<std::vector<std::vector<double>>>(m, "VectorVectorDOUBLE");
    py::bind_vector<std::vector<std::vector<uint32_t>>>\
        (m, "VectorVectorUINT32");


    py::bind_map<std::map<std::string, double>>(m, "MapStringDouble");

    // Selectors.h
    py::module env = m.def_submodule("env", "NUISANCE Environment");
    env.def("configure", &configure_environment);

    py::module sel = m.def_submodule("measurement", "Measurement Interface");

    py::class_<nuis::measurement::Record>(sel, "Record")
        .def(py::init<>())
        .def(py::init<YAML::Node>())
        .def("Summary", &nuis::measurement::Record::Summary)
        .def("Print", &nuis::measurement::Record::Print)
        .def("Reset", &nuis::measurement::Record::Reset)
        .def("GetBin", &nuis::measurement::Record::GetBin)
        .def("ResetBins", &nuis::measurement::Record::ResetBins)
        .def("FillTally", &nuis::measurement::Record::FillTally)
        .def("ResetTally", &nuis::measurement::Record::ResetTally)
        .def("GetTally", &nuis::measurement::Record::GetTally)
        .def("GetMCCount", &nuis::measurement::Record::GetMCCounts)
        .def("GetMCWeight", &nuis::measurement::Record::GetMCWeight)
        .def("GetMCError", &nuis::measurement::Record::GetMCError)
        .def("GetMCError", &nuis::measurement::Record::GetMCError)
        .def("FillBinFromIndex", &nuis::measurement::Record::FillBinFromIndex)
        .def("FillBinFromProjection", &nuis::measurement::Record::FillBinFromProjection)
        .def_readwrite("label", &nuis::measurement::Record::label)
        .def_readwrite("name", &nuis::measurement::Record::name)
        .def_readwrite("title", &nuis::measurement::Record::title)
        .def_readwrite("bin_extent_low", &nuis::measurement::Record::bin_extent_low)
        .def_readwrite("bin_extent_high", &nuis::measurement::Record::bin_extent_high)
        .def_readwrite("bin_index", &nuis::measurement::Record::bin_index)
        .def_readwrite("bin_width", &nuis::measurement::Record::bin_width)
        .def_readwrite("bin_center", &nuis::measurement::Record::bin_center)
        .def_readwrite("bin_mask", &nuis::measurement::Record::bin_mask)
        .def_readwrite("data_value", &nuis::measurement::Record::data_value)
        .def_readwrite("data_covariance", &nuis::measurement::Record::data_covariance)
        .def_readwrite("data_error", &nuis::measurement::Record::data_error)
        .def_readwrite("mc_counts", &nuis::measurement::Record::mc_counts)
        .def_readwrite("mc_weights", &nuis::measurement::Record::mc_weights)
        .def_readwrite("mc_errors", &nuis::measurement::Record::mc_errors)
        .def_readwrite("total_mc_counts", &nuis::measurement::Record::total_mc_counts)
        .def_readwrite("total_mc_weights", &nuis::measurement::Record::total_mc_weights)
        .def_readwrite("total_mc_tally", &nuis::measurement::Record::total_mc_tally)
        .def("x", &nuis::measurement::Record::GetXCenter)
        .def("y", &nuis::measurement::Record::GetYCenter)
        .def("z", &nuis::measurement::Record::GetZCenter)
        .def("mc", &nuis::measurement::Record::GetMC)
        .def("xerr", &nuis::measurement::Record::GetXErr)
        .def("yerr", &nuis::measurement::Record::GetYErr)
        .def("zerr", &nuis::measurement::Record::GetZErr)
        .def("mcerr", &nuis::measurement::Record::GetMCErr);







    py::class_<nuis::measurement::Variables>(sel, "variables")
        .def(py::init<>())
        .def("summary", &nuis::measurement::Variables::summary)
        .def_readwrite("index", &nuis::measurement::Variables::values)
        .def_readwrite("low", &nuis::measurement::Variables::low)
        .def_readwrite("high", &nuis::measurement::Variables::high)
        .def_readwrite("qualifiers", &nuis::measurement::Variables::qualifiers)
        .def_readwrite("valid", &nuis::measurement::Variables::valid)
        .def_readwrite("units", &nuis::measurement::Variables::units)
        .def_readwrite("name", &nuis::measurement::Variables::name)
        .def_readwrite("title", &nuis::measurement::Variables::title);

    py::class_<nuis::measurement::MeasurementLoader>(sel, "MeasurementLoader")
        .def(py::init<>())
        .def(py::init<YAML::Node>())
        .def("CreateRecord",
            &nuis::measurement::MeasurementLoader::CreateRecord)
        .def("Summary",
            &nuis::measurement::MeasurementLoader::summary)
        .def("FinalizeRecord",
            &nuis::measurement::MeasurementLoader::FinalizeRecord)
        .def("ProjectEvent",
            &nuis::measurement::MeasurementLoader::ProjectEvent)
        .def("FillRecordFromProj",
            &nuis::measurement::MeasurementLoader::FillRecordFromProj)
        .def("FillRecordFromEvent",
            &nuis::measurement::MeasurementLoader::FillRecordFromEvent)
        .def_readwrite("independent_variables",
            &nuis::measurement::MeasurementLoader::independent_variables )
        .def_readwrite("dependent_variables",
            &nuis::measurement::MeasurementLoader::dependent_variables );

    sel.def("CalculateRecordLikelihood",
            &nuis::measurement::CalculateRecordLikelihood);

    py::class_<nuis::measurement::HEPDataLoader,nuis::measurement::MeasurementLoader >(sel, "HEPDataLoader")
        .def(py::init<>())
        .def(py::init<YAML::Node>());
  


}
