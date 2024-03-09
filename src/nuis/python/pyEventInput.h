#pragma once

#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/python/pyYAML.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

struct pyNormalizedEventSource {
  nuis::EventSourceFactory fact;
  std::shared_ptr<HepMC3::GenRunInfo> gri;
  nuis::INormalizedEventSourcePtr evs;
  py::tuple curr_event;

  pyNormalizedEventSource(std::string filename);

  py::object first();
  py::object next();
  std::shared_ptr<HepMC3::GenRunInfo> run_info();
  double fatx();
  double sumw();
  bool good();
};

class pyNormalizedEventSource_looper {
  std::reference_wrapper<pyNormalizedEventSource> pysource;
  py::object curr_event;

public:
  pyNormalizedEventSource_looper(pyNormalizedEventSource &pyevs);
  void operator++();
  py::object const &operator*();
  bool operator!=(nuis::IEventSource_sentinel const &) const;
  bool operator==(nuis::IEventSource_sentinel const &) const;
};

pyNormalizedEventSource_looper begin(pyNormalizedEventSource &evs);
nuis::IEventSource_sentinel end(pyNormalizedEventSource &);

void pyEventInputInit(py::module &m);