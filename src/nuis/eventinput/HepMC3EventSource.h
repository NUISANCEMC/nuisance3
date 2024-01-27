#pragma once

#include "nuis/eventinput/IEventSource.h"

#include "HepMC3/ReaderFactory.h"

namespace nuis {

class HepMC3EventSource : public nuis::IEventSource {

  std::string filepath;
  std::shared_ptr<HepMC3::Reader> reader;

public:
  HepMC3EventSource(std::string const &fp) : filepath(fp) {};

  std::optional<HepMC3::GenEvent> first() {
    // reopen the file from the start and get the next event
    reader = HepMC3::deduce_reader(filepath);
    if (!reader) {
      return std::optional<HepMC3::GenEvent>();
    }
    return next();
  }

  std::optional<HepMC3::GenEvent> next() {
    if (reader->failed()) {
      return std::optional<HepMC3::GenEvent>();
    }

    HepMC3::GenEvent evt;
    reader->read_event(evt);
    if (reader->failed()) {
      return std::optional<HepMC3::GenEvent>();
    }
    return std::optional<HepMC3::GenEvent>(std::move(evt));
  }

  virtual ~HepMC3EventSource(){}
};

} // namespace nuis