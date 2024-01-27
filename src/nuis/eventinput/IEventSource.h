#pragma once

#include "HepMC3/GenEvent.h"

#include <optional>

namespace nuis {
class IEventSource {
public:
  virtual std::optional<HepMC3::GenEvent> first() = 0;
  virtual std::optional<HepMC3::GenEvent> next() = 0;

  virtual ~IEventSource(){};
};

} // namespace nuis