
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"
#include "nuis/eventinput/IEventSourceIterator.h"

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
HepMC3::GenEvent new_event(){
    return HepMC3::GenEvent();
}

std::shared_ptr<nuis::IEventSource> build_reader(std::string filename){
  EventSourceFactory fact;
  return fact.Make(filename);
}



class EventSourceWrapper {
  public:
  EventSourceWrapper(std::string filename){
    evs = fact.Make(filename);
  }

  std::optional<HepMC3::GenEvent> first(){ 
    return (*evs).first();
   }

  std::optional<HepMC3::GenEvent> next(){ 
    return (*evs).next();
  }

  EventSourceFactory fact;
  IEventSourcePtr evs;
};




PYBIND11_MODULE(pyEVENTINPUT, m) {
    m.doc() = "NUISANCE implementation in python";

    py::class_<nuis::IEventSource>(m, "IEventSource")
        .def(py::init<>())
        .def("first", &nuis::IEventSource::first)
        .def("next", &nuis::IEventSource::next);

    m.def("build_source", &build_reader);
    m.def("build_new_event", &new_event);

    py::class_<IEventSource_sentinel>(m, "EventSource_sentinel")
      .def(py::init<>());

    py::class_<IEventSource_looper>(m, "EventSource_looper")
      .def(py::init<std::shared_ptr<IEventSource>>());

    py::class_<EventSourceWrapper>(m, "EventSource")
        .def(py::init<std::string>())
        .def("first", &EventSourceWrapper::first)
        .def("next", &EventSourceWrapper::next)
        .def("__iter__", [](const EventSourceWrapper &s) {
          return py::make_iterator(begin(s.evs), end(s.evs));
          },
          py::keep_alive<0, 1>());



}
