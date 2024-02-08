#include "nuis/eventinput/IEventSource.h"

namespace nuis {

IEventSource_looper::IEventSource_looper(std::shared_ptr<IEventSource> evs)
    : source(evs) {
  curr_event = source->first();
}
void IEventSource_looper::operator++() { curr_event = source->next(); }
HepMC3::GenEvent const &IEventSource_looper::operator*() {
  return curr_event.value();
}
bool IEventSource_looper::operator!=(IEventSource_sentinel const &) const {
  return bool(curr_event);
}

bool IEventSource_looper::operator==(IEventSource_sentinel const &) const {
  return !bool(curr_event);
}

IEventSource_looper begin(IEventSourcePtr evs) {
  return IEventSource_looper(evs);
}

IEventSource_sentinel end(IEventSourcePtr) {
  return IEventSource_sentinel();
}

} // namespace nuis