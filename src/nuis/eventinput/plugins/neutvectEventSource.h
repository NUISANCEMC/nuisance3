#pragma once

#include "nuis/eventinput/IEventSource.h"

#include "TChain.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace HepMC3 {
class GenRunInfo;
}

class NeutVect;

namespace nuis {

class neutvectEventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  NeutVect *nv;

public:
  neutvectEventSource(YAML::Node const &cfg);

  std::shared_ptr<HepMC3::GenEvent> first();
  std::shared_ptr<HepMC3::GenEvent> next();

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg);

  NeutVect *neutvect(HepMC3::GenEvent const &);

  virtual ~neutvectEventSource() {}
};

IEventSourcePtr neutvectEventSource_MakeEventSource(YAML::Node const &cfg);

} // namespace nuis
