#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"
#include "nuis/eventinput/IEventSourceWrapper.h"

#include <functional>

namespace NuHepMC::FATX {
class Accumulator;
} // namespace NuHepMC::FATX
namespace NuHepMC::CrossSection::Units {
struct Unit;
} // namespace NuHepMC::CrossSection::Units

namespace nuis {

struct NormInfo {
  double fatx;
  double sumweights;
  size_t nevents;

  double fatx_per_sumweights() { return fatx / sumweights; }
};

/// An event source wrapper that keeps track of the FATX, if we cannot determine
/// how to normalize it, this is considered a resource acquisition failure
class INormalizedEventSource : public IEventSourceWrapper {

  double external_fatx;
  std::function<double(NuHepMC::CrossSection::Units::Unit const &, int)>
      external_units_scale;

  std::shared_ptr<NuHepMC::FATX::Accumulator> xs_acc;

  std::optional<EventCVWeightPair>
  process(std::shared_ptr<HepMC3::GenEvent> ev);

public:
  INormalizedEventSource(std::shared_ptr<IEventSource> evs);
  // build a normalized event source where the normalization is set by the
  // caller
  INormalizedEventSource(std::shared_ptr<IEventSource> evs, double fatx,
                         NuHepMC::CrossSection::Units::Unit const &units);
  // assume fatx specified in default units of 10^-38 cm2 / Nucleon
  INormalizedEventSource(std::shared_ptr<IEventSource> evs, double fatx);

  std::optional<EventCVWeightPair> first();
  std::optional<EventCVWeightPair> next();

  NormInfo norm_info(NuHepMC::CrossSection::Units::Unit const &units);
  virtual ~INormalizedEventSource();
};

} // namespace nuis
