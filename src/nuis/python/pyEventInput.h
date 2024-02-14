
#include "nuis/eventinput/EventSourceFactory.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace nuis;

struct pyNormalizedEventSource {
  EventSourceFactory fact;
  std::shared_ptr<HepMC3::GenRunInfo> gri;
  INormalizedEventSourcePtr evs;
  py::tuple curr_event;

  pyNormalizedEventSource(std::string filename) {
    auto resp = fact.Make(filename);
    gri = resp.first;
    evs = resp.second;
  }

  py::object first() {
    if (!evs) {
      return py::none();
    }
    auto evt = evs->first();
    curr_event = py::make_tuple(evt.value().evt, evt.value().cv_weight);
    return curr_event;
  }
  py::object next() {
    if (!evs) {
      return py::none();
    }
    auto evt = evs->next();
    curr_event = py::make_tuple(evt.value().evt, evt.value().cv_weight);
    return curr_event;
  }
  auto run_info() { return gri; }
  auto fatx() { return evs ? evs->norm_info().fatx() : 0; }
  auto sumw() { return evs ? evs->norm_info().sumweights() : 0; }
  auto good() { return bool(evs); }
};

class pyNormalizedEventSource_looper {
  std::reference_wrapper<pyNormalizedEventSource> pysource;
  py::object curr_event;

public:
  pyNormalizedEventSource_looper(pyNormalizedEventSource &pyevs)
      : pysource(pyevs) {
    curr_event = pysource.get().first();
  }
  void operator++() { curr_event = pysource.get().next(); }
  py::object const &operator*() {
    return curr_event;
  }
  bool operator!=(IEventSource_sentinel const &) const {
    return !curr_event.is(py::none());
  }
  bool operator==(IEventSource_sentinel const &) const {
    return curr_event.is(py::none());
  }
};

pyNormalizedEventSource_looper begin(pyNormalizedEventSource &evs) {
  return pyNormalizedEventSource_looper(evs);
}
IEventSource_sentinel end(pyNormalizedEventSource &) {
  return IEventSource_sentinel();
}

void init_eventinput(py::module &m) {

  py::module pyHepMC3 = py::module::import("pyHepMC3");
  m.doc() = "NUISANCE implementation in python";
  py::class_<pyNormalizedEventSource>(m, "EventSource")
      .def(py::init<std::string>())
      .def("first", &pyNormalizedEventSource::first)
      .def("next", &pyNormalizedEventSource::next)
      .def("run_info", &pyNormalizedEventSource::run_info)
      .def("fatx", &pyNormalizedEventSource::fatx)
      .def("sumw", &pyNormalizedEventSource::sumw)
      .def("good", &pyNormalizedEventSource::good)
      .def(
          "__iter__",
          [](pyNormalizedEventSource &s) {
            return py::make_iterator(begin(s), end(s));
          },
          py::keep_alive<0, 1>());
}
