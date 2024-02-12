#pragma once

#include "HepMC3/GenEvent.h"

#include <optional>

//this could be written as a template with some concrete instantiations
namespace nuis {

class IEventSource;
using IEventSourcePtr = std::shared_ptr<IEventSource>;

struct IEventSource_sentinel {};

class IEventSource_looper {
  std::shared_ptr<IEventSource> source;
  std::optional<HepMC3::GenEvent> curr_event;

public:
  IEventSource_looper(std::shared_ptr<IEventSource> evs);
  void operator++();
  HepMC3::GenEvent const &operator*();
  bool operator!=(IEventSource_sentinel const &sent) const;
  bool operator==(IEventSource_sentinel const &sent) const;
};

IEventSource_looper begin(IEventSourcePtr evs);
IEventSource_sentinel end(IEventSourcePtr evs);

class INormalizedEventSource;
using INormalizedEventSourcePtr = std::shared_ptr<INormalizedEventSource>;

struct EventCVWeightPair {
  HepMC3::GenEvent evt;
  double cv_weight;
};

class INormalizedEventSource_looper {
  std::shared_ptr<INormalizedEventSource> source;
  std::optional<EventCVWeightPair> curr_event;

public:
  INormalizedEventSource_looper(std::shared_ptr<INormalizedEventSource> evs);
  void operator++();
  EventCVWeightPair const &operator*();
  bool operator!=(IEventSource_sentinel const &sent) const;
  bool operator==(IEventSource_sentinel const &sent) const;
};

INormalizedEventSource_looper begin(INormalizedEventSourcePtr evs);
IEventSource_sentinel end(INormalizedEventSourcePtr evs);

} // namespace nuis