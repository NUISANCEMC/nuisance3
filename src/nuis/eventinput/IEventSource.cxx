#include "nuis/eventinput/IEventSource.h"
#include "nuis/eventinput/INormalizedEventSource.h"

namespace nuis {

INormalizedEventSourcePtr
IEventSource::force_fatx(double fatx,
                         NuHepMC::CrossSection::Units::Unit const &units) {
  return std::make_shared<INormalizedEventSource>(shared_from_this(), fatx,
                                                  units);
}

} // namespace nuis