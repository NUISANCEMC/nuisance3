#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"

namespace nuis {

class IEventSource {
public:
  virtual std::optional<HepMC3::GenEvent> first() = 0;
  virtual std::optional<HepMC3::GenEvent> next() = 0;

  virtual std::shared_ptr<HepMC3::GenRunInfo> run_info() = 0;

  virtual double sum_weights_so_far() = 0;

  virtual ~IEventSource(){};
};

} // namespace nuis