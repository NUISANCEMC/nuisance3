#pragma once

#include "nuis/eventinput/IEventSourceWrapper.h"

namespace nuis {

class IEventSourceRealParticleCache : public IEventSourceWrapper {
  std::vector<HepMC3::GenEvent> cached_events;

  size_t evt_ptr;

  HepMC3::GenEvent cache(HepMC3::GenEvent evt) {
    
    //get all beams, target particles, and undecayed physical and tie them
    //to a dummy vertex

  }

public:
  IEventSourceCache(std::shared_ptr<IEventSource> evs)
      : IEventSourceWrapper(evs), evt_ptr(0) {}

  std::optional<HepMC3::GenEvent> first() {
    if (cached_events.size() > 0) {
      evt_ptr = 0;
      return cached_events[evt_ptr++];
    }
    return cache(wrapped_ev_source->first());
  }

  std::optional<HepMC3::GenEvent> next() {
    if (cached_events.size() > evt_ptr) {
      return cached_events[evt_ptr++];
    }

    return cache(wrapped_ev_source->next());
  }
}

} // namespace nuis
