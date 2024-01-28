#pragma once

#include "HepMC3/GenEvent.h"

#include <functional>
#include <optional>

namespace nuis {
class IEventSource {
public:
  virtual std::optional<HepMC3::GenEvent> first() = 0;
  virtual std::optional<HepMC3::GenEvent> next() = 0;

  virtual ~IEventSource(){};
};

using IEventSourcePtr = std::shared_ptr<IEventSource>;

struct IEventSource_sentinel {};

class IEventSource_looper {
  std::shared_ptr<IEventSource> source;
  std::optional<HepMC3::GenEvent> curr_event;

public:
  IEventSource_looper() : source(nullptr), curr_event() {}
  IEventSource_looper(std::shared_ptr<IEventSource> evs) : source(evs) {
    curr_event = source->first();
  }
  void operator++() { curr_event = source->next(); }
  HepMC3::GenEvent operator*() { return curr_event.value(); }
  bool operator!=(IEventSource_sentinel const &sent) const {
    return bool(curr_event);
  }
};

inline IEventSource_looper begin(IEventSourcePtr evs) {
  return IEventSource_looper(evs);
}
inline IEventSource_sentinel end(IEventSourcePtr evs) {
  return IEventSource_sentinel();
}

class FilteredEventSource : public IEventSource {
  std::shared_ptr<IEventSource> source;
  std::function<bool(HepMC3::GenEvent const &)> filter;

public:
  FilteredEventSource(std::shared_ptr<IEventSource> evs,
                      std::function<bool(HepMC3::GenEvent const &)> filt)
      : source(evs), filter(filt){};
  std::optional<HepMC3::GenEvent> first() {
    std::optional<HepMC3::GenEvent> curr_event = source->first();
    while (bool(curr_event) && !filter(curr_event.value())) {
      curr_event = source->next();
    }
    return curr_event;
  }

  std::optional<HepMC3::GenEvent> next() {
    std::optional<HepMC3::GenEvent> curr_event = source->next();
    while (bool(curr_event) && !filter(curr_event.value())) {
      curr_event = source->next();
    }
    return curr_event;
  }
};

std::shared_ptr<FilteredEventSource>
Filter(std::shared_ptr<IEventSource> evs,
       std::function<bool(HepMC3::GenEvent const &)> filt) {
  return std::make_shared<FilteredEventSource>(evs, filt);
}

} // namespace nuis