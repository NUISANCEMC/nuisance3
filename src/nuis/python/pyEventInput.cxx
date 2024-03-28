#include "nuis/python/pyEventInput.h"

namespace py = pybind11;
using namespace nuis;

pyNormalizedEventSource::pyNormalizedEventSource(std::string const &filename) {
  auto resp = fact.make(filename);
  gri = resp.first;
  evs = resp.second;
}

pyNormalizedEventSource::pyNormalizedEventSource(YAML::Node const &node) {
  auto resp = fact.make(node);
  gri = resp.first;
  evs = resp.second;
}

py::object pyNormalizedEventSource::first() {
  if (!evs) {
    return py::none();
  }
  auto evt = evs->first();
  curr_event = py::make_tuple(evt.value().evt, evt.value().cv_weight);
  return curr_event;
}

py::object pyNormalizedEventSource::next() {
  if (!evs) {
    return py::none();
  }
  auto evt = evs->next();

  // PS : Needed this to fix event loop breaking issue
  if (!evt) {
    return py::none();
  }

  curr_event = py::make_tuple(evt.value().evt, evt.value().cv_weight);
  return curr_event;
}

std::shared_ptr<HepMC3::GenRunInfo> pyNormalizedEventSource::run_info() {
  return gri;
}

double pyNormalizedEventSource::fatx() {
  return evs ? evs->norm_info().fatx : 0;
}

double pyNormalizedEventSource::sumw() {
  return evs ? evs->norm_info().sumweights : 0;
}

bool pyNormalizedEventSource::good() { return bool(evs); }

pyNormalizedEventSource_looper::pyNormalizedEventSource_looper(
    pyNormalizedEventSource &pyevs)
    : pysource(pyevs) {
  curr_event = pysource.get().first();
}

void pyNormalizedEventSource_looper::operator++() {
  curr_event = pysource.get().next();
}

py::object const &pyNormalizedEventSource_looper::operator*() {
  return curr_event;
}
bool pyNormalizedEventSource_looper::operator!=(
    IEventSource_sentinel const &) const {
  return !curr_event.is(py::none());
}

bool pyNormalizedEventSource_looper::operator==(
    IEventSource_sentinel const &) const {
  return curr_event.is(py::none());
}

pyNormalizedEventSource_looper begin(pyNormalizedEventSource &evs) {
  return pyNormalizedEventSource_looper(evs);
}

IEventSource_sentinel end(pyNormalizedEventSource &) {
  return IEventSource_sentinel();
}

void pyEventInputInit(py::module &m) {
  py::class_<pyNormalizedEventSource>(m, "EventSource")
      .def(py::init<std::string const &>())
      .def(py::init<YAML::Node const &>())
      .def("first", &pyNormalizedEventSource::first)
      .def("next", &pyNormalizedEventSource::next)
      .def("run_info", &pyNormalizedEventSource::run_info)
      .def("fatx", &pyNormalizedEventSource::fatx)
      .def("sumw", &pyNormalizedEventSource::sumw)
      .def("__bool__", &pyNormalizedEventSource::good)
      .def(
          "__iter__",
          [](pyNormalizedEventSource &s) {
            return py::make_iterator(begin(s), end(s));
          },
          py::keep_alive<0, 1>());
}
