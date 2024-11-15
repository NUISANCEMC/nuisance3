#pragma once

#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/python/pyNUISANCE.h"

struct pyNormalizedEventSource {
  nuis::EventSourceFactory fact;
  std::shared_ptr<HepMC3::GenRunInfo> gri;
  nuis::INormalizedEventSourcePtr evs;
  pybind11::tuple curr_event;

  pyNormalizedEventSource(nuis::INormalizedEventSourcePtr);
  pyNormalizedEventSource(std::string const &filename);
  pyNormalizedEventSource(YAML::Node const &node);

  pybind11::object first();
  pybind11::object next();
  std::shared_ptr<HepMC3::GenRunInfo> run_info();
  nuis::NormInfo
  norm_info(NuHepMC::CrossSection::Units::Unit const &units) const;
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

// Unnormalized handlers
struct pyEventSource {
  nuis::EventSourceFactory fact;
  std::shared_ptr<HepMC3::GenRunInfo> gri;
  nuis::IEventSourcePtr evs;
  pybind11::tuple curr_event;

  pyEventSource(std::string const &filename);
  pyEventSource(YAML::Node const &node);

  pybind11::object first();
  pybind11::object next();
  std::shared_ptr<HepMC3::GenRunInfo> run_info();
  bool good();
};

class pyEventSource_looper {
  std::reference_wrapper<pyEventSource> pysource;
  pybind11::object curr_event;

public:
  pyEventSource_looper(pyEventSource &pyevs);
  void operator++();
  pybind11::object const &operator*();
  bool operator!=(nuis::IEventSource_sentinel const &) const;
  bool operator==(nuis::IEventSource_sentinel const &) const;
};

pyEventSource_looper begin(pyEventSource &evs);
nuis::IEventSource_sentinel end(pyEventSource &);

void pyEventInputInit(pybind11::module &m);