#pragma once

#include "nuis/eventinput/IEventSource.h"

// this is required to enable gzip reading if we built in the support
#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/ReaderFactory.h"

#include "spdlog/spdlog.h"

namespace nuis {

class HepMC3EventSource : public IEventSource {

  std::string filepath;
  std::shared_ptr<HepMC3::Reader> reader;

public:
  HepMC3EventSource(std::string const &fp) : filepath(fp){};

  std::optional<HepMC3::GenEvent> first() {

    // reopen the file from the start and get the next event
    reader = HepMC3::deduce_reader(filepath);
    if (!reader || reader->failed()) {
      spdlog::warn("Couldn't deduce reader for {} reader = {}, failed {}",
                   filepath, bool(reader), reader ? reader->failed() : false);
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

    return evt;
  }

  virtual ~HepMC3EventSource() {}
};

} // namespace nuis
