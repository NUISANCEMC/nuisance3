#pragma once

#include "nuis/eventinput/IEventSourceIterator.h"
#include "nuis/eventinput/IEventSourceWrapper.h"

namespace NuHepMC {
namespace FATX {
class Accumulator;
}
} // namespace NuHepMC

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

  std::optional<EventCVWeightPair> process(std::optional<HepMC3::GenEvent> ev);

public:
  INormalizedEventSource(std::shared_ptr<IEventSource> evs);

  std::optional<EventCVWeightPair> first();

  std::optional<EventCVWeightPair> next();

  NormInfo norm_info();
  virtual ~INormalizedEventSource();
};

} // namespace nuis
