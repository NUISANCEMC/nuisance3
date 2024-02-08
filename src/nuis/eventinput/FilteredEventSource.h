#pragma once

#include "nuis/eventinput/IEventSource.h"

#include "HepMC3/GenEvent.h"

#include <limits>
#include <optional>

namespace nuis {

template <typename SrcType, typename EvType> class FilteredEventSource_looper {
  std::reference_wrapper<SrcType> source;
  std::optional<EvType> curr_event;

public:
  FilteredEventSource_looper(SrcType &evs) : source(evs) {
    curr_event = source.get().first();
  }
  void operator++() { curr_event = source.get().next(); }
  EvType operator*() { return curr_event.value(); }
  bool operator!=(IEventSource_sentinel const &) const {
    return bool(curr_event);
  }
};

template <typename EvtSrcT, typename Key> class MultiFilteredEventSource {

  EvtSrcT fsource;
  std::map<Key, std::function<bool(HepMC3::GenEvent const &)>> filters;

  std::vector<Key> filter(std::optional<HepMC3::GenEvent> const &evt) {
    if (!evt) {
      return std::vector<Key>{};
    }

    std::vector<Key> passed_filters;
    for (auto &[k, filt] : filters) {
      if (filt(evt.value())) {
        passed_filters.push_back(k);
      }
    }
    return passed_filters;
  }

public:
  MultiFilteredEventSource(
      EvtSrcT evs,
      std::map<Key, std::function<bool(HepMC3::GenEvent const &)>> filts)
      : fsource(evs), filters(filts){};

  struct FilteredEvent {
    std::vector<Key> passed_filters;
    HepMC3::GenEvent event;
  };

  std::optional<FilteredEvent> first() {
    std::optional<HepMC3::GenEvent> curr_event = fsource.first();
    std::vector<Key> passed_filters = filter(curr_event);

    while (bool(curr_event) && !passed_filters.size()) {
      curr_event = fsource.next();
      passed_filters = filter(curr_event);
    }
    return bool(curr_event) ? FilteredEvent{passed_filters, curr_event.value()}
                            : std::optional<FilteredEvent>();
  }

  std::optional<FilteredEvent> next() {
    std::optional<HepMC3::GenEvent> curr_event = fsource.next();
    std::vector<Key> passed_filters = filter(curr_event);

    while (bool(curr_event) && !passed_filters.size()) {
      curr_event = fsource.next();
      passed_filters = filter(curr_event);
    }
    return bool(curr_event) ? FilteredEvent{passed_filters, curr_event.value()}
                            : std::optional<FilteredEvent>();
  }

  FilteredEventSource_looper<MultiFilteredEventSource<EvtSrcT, Key>,
                             FilteredEvent>
  begin() {
    return FilteredEventSource_looper<MultiFilteredEventSource<EvtSrcT, Key>,
                                      FilteredEvent>(*this);
  }
  IEventSource_sentinel end() { return IEventSource_sentinel(); }

  ~MultiFilteredEventSource() {}
};

class FilteredEventSource : public IEventSource {
  std::shared_ptr<IEventSource> source;
  std::vector<std::function<bool(HepMC3::GenEvent const &)>> filters;

  size_t nread, natmost;

  bool filter(HepMC3::GenEvent const &ev) {
    for (auto &f : filters) {
      if (!f(ev)) {
        return false;
      }
    }
    return true;
  }

public:
  FilteredEventSource(std::shared_ptr<IEventSource> evs)
      : source(evs), nread{0}, natmost{std::numeric_limits<size_t>::max()} {}
  FilteredEventSource(std::shared_ptr<IEventSource> evs,
                      std::function<bool(HepMC3::GenEvent const &)> filt)
      : source(evs), nread{0}, natmost{std::numeric_limits<size_t>::max()} {
    filters.push_back(std::move(filt));
  }
  FilteredEventSource(
      std::shared_ptr<IEventSource> evs,
      std::vector<std::function<bool(HepMC3::GenEvent const &)>> filts)
      : source(evs), filters(std::move(filts)) {}

  std::optional<HepMC3::GenEvent> first() {
    std::optional<HepMC3::GenEvent> curr_event = source->first();
    nread = 0;
    if (curr_event) {
      nread++;
    }
    while (bool(curr_event) && !filter(curr_event.value())) {
      if (nread >= natmost) {
        return std::optional<HepMC3::GenEvent>();
      }
      curr_event = source->next();
      if (curr_event) {
        nread++;
      }
    }
    return curr_event;
  }

  std::optional<HepMC3::GenEvent> next() {

    if (nread >= natmost) {
      return std::optional<HepMC3::GenEvent>();
    }

    std::optional<HepMC3::GenEvent> curr_event = source->next();
    if (curr_event) {
      nread++;
    }
    while (bool(curr_event) && !filter(curr_event.value())) {
      if (nread >= natmost) {
        return std::optional<HepMC3::GenEvent>();
      }
      curr_event = source->next();
      if (curr_event) {
        nread++;
      }
    }

    return curr_event;
  }

  FilteredEventSource sel(std::function<bool(HepMC3::GenEvent const &)> filt) {
    auto fevs = FilteredEventSource(source, filters);
    fevs.natmost = natmost;
    fevs.filters.push_back(std::move(filt));

    return fevs;
  }

  FilteredEventSource atmost(size_t nmost) {
    auto fevs = FilteredEventSource(source, filters);
    fevs.natmost = nmost;
    return fevs;
  }

  size_t events_read() { return nread; }

  template <typename Key>
  MultiFilteredEventSource<FilteredEventSource, Key>
  multisel(std::map<Key, std::function<bool(HepMC3::GenEvent const &)>> filts) {
    auto fevs = FilteredEventSource(source, filters);
    fevs.natmost = natmost;
    return MultiFilteredEventSource<FilteredEventSource, Key>(fevs, filts);
  }

  FilteredEventSource_looper<FilteredEventSource, HepMC3::GenEvent> begin() {
    return FilteredEventSource_looper<FilteredEventSource, HepMC3::GenEvent>(
        *this);
  }
  IEventSource_sentinel end() { return IEventSource_sentinel(); }

  ~FilteredEventSource() {}
};

FilteredEventSource from(std::shared_ptr<IEventSource> evs) {
  return FilteredEventSource(evs);
}

} // namespace nuis