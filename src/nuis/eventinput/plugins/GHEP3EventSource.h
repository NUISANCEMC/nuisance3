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

namespace genie {
class NtpMCEventRecord;
class GEVGDriver;
class EventRecord;
class Spline;
} // namespace genie

namespace nuis {

class GHEP3EventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  genie::NtpMCEventRecord *ntpl;

  std::string EventGeneratorListName;
  std::unordered_map<
      int, std::unordered_map<int, std::unique_ptr<genie::GEVGDriver>>>
      EvGens;

  genie::Spline const *GetSpline(int tgtpdg, int nupdg);

public:
  GHEP3EventSource(YAML::Node const &cfg);

  std::optional<HepMC3::GenEvent> first();

  std::optional<HepMC3::GenEvent> next();

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg);

  genie::EventRecord const *EventRecord(HepMC3::GenEvent const &ev);

  virtual ~GHEP3EventSource() {}
};

} // namespace nuis
