#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"

#include "HepMC3/GenParticle.h"

NEW_NUISANCE_EXCEPT(EventMomentumUnitNotMeV);

namespace nuis {

std::optional<EventCVWeightPair>
INormalizedEventSource::process(std::shared_ptr<HepMC3::GenEvent> ev) {
  if (!ev) {
    return std::optional<EventCVWeightPair>();
  }

#ifndef NUIS_NDEBUG
  if (NuHepMC::Event::ToMeVFactor(*ev) != 1) {
    log_critical(
        "[INormalizedEventSource]: Processing event not in MeV. This breaks "
        "a critical contract to users, fix the underlying IEventSource.");
    throw EventMomentumUnitNotMeV();
  }

  auto fsprot = NuHepMC::Event::GetParticles_AllRealFinalState(*ev, {
                                                                        2212,
                                                                    });
  if (fsprot.size()) {
    if (fsprot.front()->momentum().m() < 10) {
      log_critical(
          "[INormalizedEventSource]: Processing event with a real final "
          "state proton with a reported mass of {} MeV, the units look "
          "incorrectly set. This breaks "
          "a critical contract to users, fix the underlying IEventSource.");
      throw EventMomentumUnitNotMeV();
    }
  }
#endif

  return EventCVWeightPair{ev, xs_acc->process(*ev)};
}

INormalizedEventSource::INormalizedEventSource(
    std::shared_ptr<IEventSource> evs)
    : IEventSourceWrapper(evs) {}

std::optional<EventCVWeightPair> INormalizedEventSource::first() {
  if (!wrapped_ev_source) {
    return std::optional<EventCVWeightPair>();
  }

  try {
    xs_acc =
        NuHepMC::FATX::MakeAccumulator(wrapped_ev_source->first()->run_info());
  } catch (NuHepMC::except const &ex) {
    log_warn("INormalizedEventSource::first failed to determine cross-section "
             "scaling information from event stream. If you need to read this "
             "file, request an unnormalized EventSource.");
    return std::optional<EventCVWeightPair>();
  }
  return process(wrapped_ev_source->first());
}

std::optional<EventCVWeightPair> INormalizedEventSource::next() {
  return process(wrapped_ev_source->next());
}

NormInfo INormalizedEventSource::norm_info(
    NuHepMC::CrossSection::Units::Unit const &units) {
  return {xs_acc->fatx(units), xs_acc->sumweights(), xs_acc->events()};
}

INormalizedEventSource::~INormalizedEventSource() {}

} // namespace nuis
