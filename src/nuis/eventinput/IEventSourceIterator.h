#pragma once

#include "HepMC3/GenEvent.h"

#include <optional>

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
  HepMC3::GenEvent operator*();
  bool operator!=(IEventSource_sentinel const &sent) const;
};

IEventSource_looper begin(IEventSourcePtr evs);
IEventSource_sentinel end(IEventSourcePtr evs);

} // namespace nuis