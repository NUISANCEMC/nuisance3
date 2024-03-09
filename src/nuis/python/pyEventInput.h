#pragma once

#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/python/pyNUISANCE.h"

struct pyNormalizedEventSource {
  nuis::EventSourceFactory fact;
  std::shared_ptr<HepMC3::GenRunInfo> gri;
  nuis::INormalizedEventSourcePtr evs;
  pybind11::tuple curr_event;

  pyNormalizedEventSource(std::string filename);

  pybind11::object first();
  pybind11::object next();
  std::shared_ptr<HepMC3::GenRunInfo> run_info();
  double fatx();
  double sumw();
  bool good();
};

class pyNormalizedEventSource_looper {
  std::reference_wrapper<pyNormalizedEventSource> pysource;
  pybind11::object curr_event;

public:
  pyNormalizedEventSource_looper(pyNormalizedEventSource &pyevs);
  void operator++();
  pybind11::object const &operator*();
  bool operator!=(nuis::IEventSource_sentinel const &) const;
  bool operator==(nuis::IEventSource_sentinel const &) const;
};

pyNormalizedEventSource_looper begin(pyNormalizedEventSource &evs);
nuis::IEventSource_sentinel end(pyNormalizedEventSource &);

void pyEventInputInit(pybind11::module &m);