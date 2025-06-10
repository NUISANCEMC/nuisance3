#include "nuis/eventinput/NormalizedEventSource.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"

#include "HepMC3/GenParticle.h"

DECLARE_NUISANCE_EXCEPT(EventMomentumUnitNotMeV);
DECLARE_NUISANCE_EXCEPT(InvalidXSUnitsForNormalization);

namespace nuis {

std::optional<EventCVWeightPair>
NormalizedEventSource::process(std::shared_ptr<HepMC3::GenEvent> ev) {
  if (!ev) {
    return std::optional<EventCVWeightPair>();
  }

#ifndef NUIS_NDEBUG
  if (NuHepMC::Event::ToMeVFactor(*ev) != 1) {
    log_critical(
        "[NormalizedEventSource]: Processing event not in MeV. This breaks "
        "a critical contract to users, fix the underlying IEventSource.");
    throw EventMomentumUnitNotMeV();
  }

  auto fsprot = NuHepMC::Event::GetParticles_AllRealFinalState(*ev, {
                                                                        2212,
                                                                    });
  if (fsprot.size()) {
    if (fsprot.front()->momentum().m() < 10) {
      log_critical(
          "[NormalizedEventSource]: Processing event with a real final "
          "state proton with a reported mass of {} MeV, the units look "
          "incorrectly set. This breaks "
          "a critical contract to users, fix the underlying IEventSource.");
      throw EventMomentumUnitNotMeV();
    }
  }
#endif

  return EventCVWeightPair{ev, xs_acc->process(*ev)};
}

NormalizedEventSource::NormalizedEventSource(
    std::shared_ptr<IEventSource> evs)
    : IEventSourceWrapper(evs), external_fatx{0xdeadbeef} {}

NormalizedEventSource::NormalizedEventSource(
    std::shared_ptr<IEventSource> evs, double fatx,
    NuHepMC::CrossSection::Units::Unit const &input_units)
    : IEventSourceWrapper(evs), external_fatx{fatx} {

  external_units_scale = [=](NuHepMC::CrossSection::Units::Unit const &to_units,
                             int target_A) {
    using namespace NuHepMC::CrossSection::Units;
    static std::map<Scale, double> const xsunit_factors = {
        {Scale::pb, pb},
        {Scale::cm2, cm2},
        {Scale::cm2_ten38, cm2_ten38},
    };

    // pb -> cm2 : 1E-36
    // 1      1E36
    //   1 / 1E36 = 1E-36!
    double unit_sf = xsunit_factors.at(input_units.scale) /
                     xsunit_factors.at(to_units.scale);

    if (input_units.tgtscale == to_units.tgtscale) {
      return unit_sf;
    }

    if ((input_units.tgtscale == TargetScale::PerAtom) &&
        (to_units.tgtscale == TargetScale::PerNucleon)) {
      return unit_sf / double(target_A);
    } else if ((input_units.tgtscale == TargetScale::PerNucleon) &&
               (to_units.tgtscale == TargetScale::PerAtom)) {
      return unit_sf * double(target_A);
    } else {
      throw InvalidXSUnitsForNormalization()
          << "Cannot convert input cross section units: " << input_units
          << " to requested units: " << to_units
          << " for external fatx scaling without additional information, "
             "please convert the fatx to the required units manually and "
             "specify those units as the input units when creating the "
             "NormalizedEventSource.";
    }
  };
}

NormalizedEventSource::NormalizedEventSource(
    std::shared_ptr<IEventSource> evs, double fatx)
    : NormalizedEventSource(
          evs, fatx, NuHepMC::CrossSection::Units::cm2ten38_PerNucleon) {}

std::optional<EventCVWeightPair> NormalizedEventSource::first() {
  if (!wrapped_ev_source) {
    return std::optional<EventCVWeightPair>();
  }

  try {
    if (external_fatx != 0xdeadbeef) {
      xs_acc = NuHepMC::FATX::MakeAccumulator("Dummy");
    } else {
      xs_acc = NuHepMC::FATX::MakeAccumulator(
          wrapped_ev_source->first()->run_info());
    }
  } catch (NuHepMC::except const &ex) {
    log_warn("NormalizedEventSource::first failed to determine cross-section "
             "scaling information from event stream. If you need to read this "
             "file, request an unnormalized EventSource: {}", ex.what());
    return std::optional<EventCVWeightPair>();
  }
  return process(wrapped_ev_source->first());
}

std::optional<EventCVWeightPair> NormalizedEventSource::next() {
  return process(wrapped_ev_source->next());
}

NormInfo NormalizedEventSource::norm_info(
    NuHepMC::CrossSection::Units::Unit const &units) {

  if (external_fatx != 0xdeadbeef) {
    return {external_fatx *
                external_units_scale(units, xs_acc->TargetTotalNucleons()),
            xs_acc->sumweights(), xs_acc->events()};
  } else {
    return {xs_acc->fatx(units), xs_acc->sumweights(), xs_acc->events()};
  }
}

NormalizedEventSource::~NormalizedEventSource() {}

} // namespace nuis
