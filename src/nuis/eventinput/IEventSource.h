#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"

#include "nuis/log.h"

namespace NuHepMC::CrossSection::Units {
struct Unit;
} // namespace NuHepMC::CrossSection::Units

namespace nuis {

class IEventSource : public std::enable_shared_from_this<IEventSource>,
                     public nuis_named_log("EventInput") {
public:
  virtual std::shared_ptr<HepMC3::GenEvent> first() = 0;
  virtual std::shared_ptr<HepMC3::GenEvent> next() = 0;

  // Allows you to force the flux-averaged total cross section
  NormalizedEventSourcePtr force_fatx(
      double fatx, NuHepMC::CrossSection::Units::Unit const &units);

  virtual ~IEventSource(){};
};

} // namespace nuis