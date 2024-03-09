#include "nuis/eventinput/IEventSourceWrapper.h"

#include "nuis/log.txx"

namespace nuis {

IEventSourceWrapper::IEventSourceWrapper(std::shared_ptr<IEventSource> evs)
    : wrapped_ev_source(evs) {}

std::shared_ptr<IEventSource> IEventSourceWrapper::unwrap() {
  return wrapped_ev_source;
}

IEventSourceWrapper::~IEventSourceWrapper() {}

} // namespace nuis
