#include "nuis/eventinput/IEventSource.h"
#include "nuis/eventinput/NormalizedEventSource.h"

namespace nuis {

NormalizedEventSourcePtr
IEventSource::force_fatx(double fatx,
                         NuHepMC::CrossSection::Units::Unit const &units) {
  return std::make_shared<NormalizedEventSource>(shared_from_this(), fatx,
                                                  units);
}

} // namespace nuis