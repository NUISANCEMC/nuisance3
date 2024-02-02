#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"

namespace nuis {

class IEventSource {
public:
  virtual std::optional<HepMC3::GenEvent> first(){ return HepMC3::GenEvent(); };
  virtual std::optional<HepMC3::GenEvent> next(){ return HepMC3::GenEvent(); };

  virtual ~IEventSource(){};
  std::string filenames;

};

} // namespace nuis