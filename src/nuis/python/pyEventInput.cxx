#include "nuis/python/pyEventInput.h"

#include "NuHepMC/UnitsUtils.hxx"

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

NormInfo pyNormalizedEventSource::norm_info(
    NuHepMC::CrossSection::Units::Unit const &units) const {
  return evs ? evs->norm_info(units) : NormInfo{};
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



// Unnormalized bindings

pyEventSource::pyEventSource(std::string const &filename) {
  auto resp = fact.make_unnormalized(filename);
  gri = resp.first;
  evs = resp.second;
}

pyEventSource::pyEventSource(YAML::Node const &node) {
  auto resp = fact.make_unnormalized(node);
  gri = resp.first;
  evs = resp.second;
}

py::object pyEventSource::first() {
  if (!evs) {
    return py::none();
  }
  auto evt = evs->first();
  curr_event = py::make_tuple(evt, 1.0);
  return curr_event;
}

py::object pyEventSource::next() {
  if (!evs) {
    return py::none();
  }
  auto evt = evs->next();

  // PS : Needed this to fix event loop breaking issue
  if (!evt) {
    return py::none();
  }
  curr_event = py::make_tuple(evt, 1.0);
  return curr_event;
}

std::shared_ptr<HepMC3::GenRunInfo> pyEventSource::run_info() {
  return gri;
}

bool pyEventSource::good() { return bool(evs); }

pyEventSource_looper::pyEventSource_looper(
    pyEventSource &pyevs)
    : pysource(pyevs) {
  curr_event = pysource.get().first();
}

void pyEventSource_looper::operator++() {
  curr_event = pysource.get().next();
}

py::object const &pyEventSource_looper::operator*() {
  return curr_event;
}
bool pyEventSource_looper::operator!=(
    IEventSource_sentinel const &) const {
  return !curr_event.is(py::none());
}

bool pyEventSource_looper::operator==(
    IEventSource_sentinel const &) const {
  return curr_event.is(py::none());
}

pyEventSource_looper begin(pyEventSource &evs) {
  return pyEventSource_looper(evs);
}

IEventSource_sentinel end(pyEventSource &) {
  return IEventSource_sentinel();
}


void pyEventInputInit(py::module &m) {

  py::class_<NormInfo>(m, "NormInfo")
      .def_readonly("fatx", &NormInfo::fatx)
      .def_readonly("sumweights", &NormInfo::sumweights)
      .def_readonly("nevents", &NormInfo::nevents)
      .def("fatx_per_sumweights", &NormInfo::fatx_per_sumweights);

  py::class_<pyNormalizedEventSource>(m, "EventSource")
      .def(py::init<std::string const &>())
      .def(py::init<YAML::Node const &>())
      .def("first", &pyNormalizedEventSource::first)
      .def("next", &pyNormalizedEventSource::next)
      .def("run_info", &pyNormalizedEventSource::run_info)
      .def("norm_info", &pyNormalizedEventSource::norm_info,
           py::arg("units") = NuHepMC::CrossSection::Units::cm2ten38_PerNucleon)
      .def("__bool__", &pyNormalizedEventSource::good)
      .def(
          "__iter__",
          [](pyNormalizedEventSource &s) {
            return py::make_iterator(begin(s), end(s));
          },
          py::keep_alive<0, 1>());

  py::class_<pyEventSource>(m, "UnNormalizedEventSource")
      .def(py::init<std::string const &>())
      .def(py::init<YAML::Node const &>())
      .def("first", &pyEventSource::first)
      .def("next", &pyEventSource::next)
      .def("run_info", &pyEventSource::run_info)
      .def("__bool__", &pyEventSource::good)
      .def(
          "__iter__",
          [](pyEventSource &s) {
            return py::make_iterator(begin(s), end(s));
          },
          py::keep_alive<0, 1>());

      
}
