#pragma once

#include "nuis/eventinput/IEventSource.h"

// this is required to enable gzip reading if we built in the support
#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/ReaderFactory.h"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <fstream>

namespace nuis {

class HepMC3EventSource : public IEventSource {

  std::filesystem::path filepath;
  std::shared_ptr<HepMC3::Reader> reader;

public:
  HepMC3EventSource(std::filesystem::path const &fp) : filepath(fp){};

  std::optional<HepMC3::GenEvent> first() {

    // refuse to read ROOT files as the reader has a bug in it
    if (!std::filesystem::exists(filepath)) {
      spdlog::warn("HepMC3EventSource ignoring non-existant path {}",
                   filepath.native());
      return std::optional<HepMC3::GenEvent>();
    }
    std::ifstream fin(filepath);
    char magicbytes[5];
    fin.read(magicbytes, 4);
    magicbytes[4] = '\0';
    if (std::string(magicbytes) == "root") {
      spdlog::warn("HepMC3EventSource ignoring ROOT file {}", filepath.native(),
                   magicbytes);
      return std::optional<HepMC3::GenEvent>();
    }

    // reopen the file from the start and get the next event
    reader = HepMC3::deduce_reader(filepath);
    if (!reader || reader->failed()) {
      spdlog::warn("Couldn't deduce reader for {} reader = {}, failed {}",
                   filepath.native(), bool(reader), reader ? reader->failed() : false);
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
