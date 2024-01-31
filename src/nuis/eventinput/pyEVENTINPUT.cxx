
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"

#include "spdlog/spdlog.h"


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, double>);

#include "yaml-cpp/yaml.h"

namespace py = pybind11;
#include "spdlog/spdlog.h"
using namespace nuis;


// Hack for now to make a local event
// HepMC3::GenEvent new_event(){
    // return HepMC3::GenEvent();
// }

std::shared_ptr<nuis::IEventSource> build_reader(std::string filename){
  EventSourceFactory fact;
  return fact.Make(filename);
}

PYBIND11_MODULE(pyEVENTINPUT, m) {
    m.doc() = "NUISANCE implementation in python";

    py::module gen = m.def_submodule("gen", "NUISANCE GEN");

    py::class_<nuis::IEventSource>(gen, "IEventSource")
        .def(py::init<>())
        .def("first", &nuis::IEventSource::first)
        .def("next", &nuis::IEventSource::next);

    gen.def("build_source", &build_reader);
    // gen.def("build_new_event", &new_event);

}
