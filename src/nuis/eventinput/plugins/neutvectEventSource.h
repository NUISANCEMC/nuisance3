#pragma once

#include "nuis/eventinput/IEventSource.h"

#include "neutvect.h"

#include "TChain.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace HepMC3 {
class GenRunInfo;
}

namespace nuis {

class neutvectEventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  NeutVect *nv;

  void CheckAndAddPath(std::filesystem::path filepath);

public:
  neutvectEventSource(YAML::Node const &cfg);

  std::optional<HepMC3::GenEvent> first();
  std::optional<HepMC3::GenEvent> next();

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg);

  NeutVect *neutvect(HepMC3::GenEvent const &);

  virtual ~neutvectEventSource() {}
};

} // namespace nuis
