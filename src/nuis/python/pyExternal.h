#pragma once
#include <string>

#include "nuis/eventinput/EventSourceFactory.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace nuis;

    using ProjectFunc =
        std::function<double(HepMC3::GenEvent const &)>;

    using FilterFunc =
        std::function<int(HepMC3::GenEvent const &)>;

ProjectFunc GetProjectionFunc(std::string key) {
    auto f = ProSelecta::Get().GetProjectionFunction(key,
            ProSelecta::Interpreter::kCling);
    if (!f){
        std::cerr << "Failed to find function : " << key << std::endl;
        abort();
    }
    return f;
}

FilterFunc GetFilterFunc(std::string key) {
    auto f = ProSelecta::Get().GetFilterFunction(key,
            ProSelecta::Interpreter::kCling);
    if (!f){
        std::cerr << "Failed to find function : " << key << std::endl;
        abort();
    }
    return f;
}

void init_external(py::module &m) {

    py::module pyHepMC3 = py::module::import("pyHepMC3");
    auto m_ps = m.def_submodule("ps", "ProSelectra interface");

    m_ps.def("LoadFile", [](std::string analysis) {
        ProSelecta::Get().LoadFile(analysis.c_str(),
        ProSelecta::Interpreter::kCling);
    });

    m_ps.def("LoadText", [](std::string analysis) {
        ProSelecta::Get().LoadText(analysis.c_str(),
            ProSelecta::Interpreter::kCling);
    });

    // Heretic!
    m_ps.def("CreateFilter", [](std::string filter, std::string analysis) {
        std::string filter_analysis = "int " +
            filter + "(HepMC3::GenEvent const &ev) {";
        filter_analysis += analysis;
        filter_analysis += "}";
        ProSelecta::Get().LoadText(filter_analysis.c_str(),
            ProSelecta::Interpreter::kCling);
    });

    m_ps.def("CreateProjection", [](std::string projection,
        std::string analysis) {
        std::string projection_analysis = "double " +
            projection + "(HepMC3::GenEvent const &ev) {";
        projection_analysis += analysis;
        projection_analysis += "}";
        ProSelecta::Get().LoadText(projection_analysis.c_str(),
            ProSelecta::Interpreter::kCling);
    });

    m_ps.def("AddIncludePath", [](std::string analysis) {
        ProSelecta::Get().AddIncludePath(analysis.c_str());
    });

    auto m_ps_filter = m_ps.def_submodule("filter",
        "ProSelectra filter interface");

    m_ps_filter.def("__getattr__", &GetFilterFunc);

    auto m_ps_project = m_ps.def_submodule("project",
        "ProSelectra projection interface");

    m_ps_project.def("__getattr__", &GetProjectionFunc);

}

