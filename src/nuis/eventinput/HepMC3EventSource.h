#pragma once

#include "nuis/eventinput/IEventSource.h"

#include <filesystem>

namespace HepMC3 {
class Reader;
}

namespace nuis {

class HepMC3EventSource : public IEventSource {

  std::filesystem::path filepath;
  std::shared_ptr<HepMC3::Reader> reader;

public:
  HepMC3EventSource(std::filesystem::path const &fp);

  std::optional<HepMC3::GenEvent> first();
  std::optional<HepMC3::GenEvent> next();

  virtual ~HepMC3EventSource();
};

} // namespace nuis
