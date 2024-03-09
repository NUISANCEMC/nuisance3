#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"

#include "nuis/log.h"

namespace nuis {

class IEventSource : public nuis_named_log("EventInput") {
public:
  virtual std::optional<HepMC3::GenEvent> first() = 0;
  virtual std::optional<HepMC3::GenEvent> next() = 0;

  virtual ~IEventSource(){};
};

} // namespace nuis