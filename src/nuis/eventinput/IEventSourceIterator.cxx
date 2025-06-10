#include "nuis/eventinput/IEventSource.h"
#include "nuis/eventinput/NormalizedEventSource.h"

namespace nuis {

IEventSource_looper::IEventSource_looper(std::shared_ptr<IEventSource> evs)
    : source(evs) {
  if (source) {
    curr_event = source->first();
  }
}
void IEventSource_looper::operator++() { curr_event = source->next(); }
HepMC3::GenEvent const &IEventSource_looper::operator*() { return *curr_event; }
bool IEventSource_looper::operator!=(IEventSource_sentinel const &) const {
  return bool(curr_event);
}

bool IEventSource_looper::operator==(IEventSource_sentinel const &) const {
  return !bool(curr_event);
}

IEventSource_looper begin(IEventSourcePtr evs) {
  return IEventSource_looper(evs);
}

IEventSource_sentinel end(IEventSourcePtr) { return IEventSource_sentinel(); }

NormalizedEventSource_looper::NormalizedEventSource_looper(
    std::shared_ptr<NormalizedEventSource> evs)
    : source(evs) {
  if (source) {
    curr_event = source->first();
  }
}
void NormalizedEventSource_looper::operator++() {
  curr_event = source->next();
}
EventCVWeightPair const &NormalizedEventSource_looper::operator*() {
  return curr_event.value();
}
bool NormalizedEventSource_looper::operator!=(
    IEventSource_sentinel const &) const {
  return bool(curr_event);
}

bool NormalizedEventSource_looper::operator==(
    IEventSource_sentinel const &) const {
  return !bool(curr_event);
}

NormalizedEventSource_looper begin(NormalizedEventSourcePtr evs) {
  return NormalizedEventSource_looper(evs);
}

IEventSource_sentinel end(NormalizedEventSourcePtr) {
  return IEventSource_sentinel();
}

} // namespace nuis