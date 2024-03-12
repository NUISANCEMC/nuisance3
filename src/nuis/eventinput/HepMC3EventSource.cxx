#include "nuis/eventinput/HepMC3EventSource.h"

// this is required to enable gzip reading if we built in the support
#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/ReaderFactory.h"

#include "nuis/log.txx"

#include <fstream>

namespace nuis {

HepMC3EventSource::HepMC3EventSource(std::filesystem::path const &fp)
    : filepath(fp){};

std::shared_ptr<HepMC3::GenEvent> HepMC3EventSource::first() {

  // refuse to read ROOT files as the reader has a bug in it
  if (!std::filesystem::exists(filepath)) {
    log_warn("HepMC3EventSource ignoring non-existant path {}",
             filepath.native());
    return nullptr;
  }
  std::ifstream fin(filepath);
  char magicbytes[5];
  fin.read(magicbytes, 4);
  magicbytes[4] = '\0';
  if (std::string(magicbytes) == "root") {
    log_warn("HepMC3EventSource ignoring ROOT file {}", filepath.native(),
             magicbytes);
    return nullptr;
  }

  // reopen the file from the start and get the next event
  reader = HepMC3::deduce_reader(filepath);
  if (!reader || reader->failed()) {
    log_warn("Couldn't deduce reader for {} reader = {}, failed {}",
             filepath.native(), bool(reader),
             reader ? reader->failed() : false);
    return nullptr;
  }
  return next();
}

std::shared_ptr<HepMC3::GenEvent> HepMC3EventSource::next() {
  if (reader->failed()) {
    return nullptr;
  }

  auto evt = std::make_shared<HepMC3::GenEvent>();
  reader->read_event(*evt);
  if (reader->failed()) {
    return nullptr;
  }

  evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
  return evt;
}

HepMC3EventSource::~HepMC3EventSource() {}

} // namespace nuis
