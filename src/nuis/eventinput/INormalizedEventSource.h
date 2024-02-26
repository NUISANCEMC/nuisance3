#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"
#include "nuis/eventinput/IEventSourceWrapper.h"

#include "NuHepMC/FATXUtils.hxx"

namespace nuis {

struct NormInfo {
  double fatx;
  double sumweights;
  size_t nevents;
};

/// An event source wrapper that keeps track of the FATX, if we cannot determine
/// how to normalize it, this is considered a resource acquisition failure
class INormalizedEventSource : public IEventSourceWrapper {

  std::shared_ptr<NuHepMC::FATX::Accumulator> xs_acc;

  std::optional<EventCVWeightPair> process(std::optional<HepMC3::GenEvent> ev) {
    if (!ev) {
      return std::optional<EventCVWeightPair>();
    }
    double cvw = xs_acc->process(ev.value());
    return EventCVWeightPair{std::move(ev.value()), cvw};
  }

public:
  INormalizedEventSource(std::shared_ptr<IEventSource> evs)
      : IEventSourceWrapper(evs) {}

  std::optional<EventCVWeightPair> first() {
    if (!wrapped_ev_source) {
      return std::optional<EventCVWeightPair>();
    }

    try {
      xs_acc = NuHepMC::FATX::MakeAccumulator(
          wrapped_ev_source->first().value().run_info());
    } catch (NuHepMC::except const &ex) {
      return std::optional<EventCVWeightPair>();
    }
    return process(wrapped_ev_source->first());
  }

  std::optional<EventCVWeightPair> next() {
    return process(wrapped_ev_source->next());
  }

  NormInfo norm_info() {
    return {xs_acc->fatx(), xs_acc->sumweights(), xs_acc->events()};
  }
};

} // namespace nuis
