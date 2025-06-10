#pragma once

#include "HepMC3/GenEvent.h"

#include <memory>
#include <optional>

// this could be written as a template with some concrete instantiations
namespace nuis {

class IEventSource;
using IEventSourcePtr = std::shared_ptr<IEventSource>;

struct IEventSource_sentinel {};

class IEventSource_looper {
  IEventSourcePtr source;
  std::shared_ptr<HepMC3::GenEvent> curr_event;

public:
  IEventSource_looper(IEventSourcePtr evs);
  void operator++();
  HepMC3::GenEvent const &operator*();
  bool operator!=(IEventSource_sentinel const &sent) const;
  bool operator==(IEventSource_sentinel const &sent) const;
};

IEventSource_looper begin(IEventSourcePtr evs);
IEventSource_sentinel end(IEventSourcePtr evs);

class NormalizedEventSource;
using NormalizedEventSourcePtr = std::shared_ptr<NormalizedEventSource>;

struct EventCVWeightPair {
  std::shared_ptr<HepMC3::GenEvent> evt;
  double cv_weight;
};

class NormalizedEventSource_looper {
  NormalizedEventSourcePtr source;
  std::optional<EventCVWeightPair> curr_event;

public:
  NormalizedEventSource_looper(NormalizedEventSourcePtr evs);
  void operator++();
  EventCVWeightPair const &operator*();
  bool operator!=(IEventSource_sentinel const &sent) const;
  bool operator==(IEventSource_sentinel const &sent) const;
};

NormalizedEventSource_looper begin(NormalizedEventSourcePtr evs);
IEventSource_sentinel end(NormalizedEventSourcePtr evs);

} // namespace nuis