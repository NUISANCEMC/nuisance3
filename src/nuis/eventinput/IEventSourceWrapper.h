#pragma once

#include "nuis/eventinput/IEventSource.h"

#include "nuis/log.h"

namespace nuis {

// ABC for EventSource wrappers that want to give access to the underlying
// lowest level event source
// See INormalizedEventSource for example usage
class IEventSourceWrapper
    : public nuis_named_log("EventInput") {
protected:
  std::shared_ptr<IEventSource> wrapped_ev_source;

public:
  IEventSourceWrapper(std::shared_ptr<IEventSource> evs);

  template <typename T> std::shared_ptr<T> as() {
    return std::dynamic_pointer_cast<T>(wrapped_ev_source);
  }

  std::shared_ptr<IEventSource> unwrap();

  virtual ~IEventSourceWrapper();
};

using IWrappedEventSourcePtr = std::shared_ptr<IEventSourceWrapper>;

} // namespace nuis
