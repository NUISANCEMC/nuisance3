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

  genie::EventRecord const *first_GHEPevent();
  genie::EventRecord const *next_GHEPevent();

public:
  class XSSplines {
    std::string EventGeneratorListName;
    std::unordered_map<
        int, std::unordered_map<int, std::unique_ptr<genie::GEVGDriver>>>
        EvGens;

    genie::GEVGDriver &EVGDriver(int tgtpdg, int nupdg);

  public:
    XSSplines(std::string const &tune, std::string const &event_generator_list,
              std::string const &spline_file);

    genie::Spline const *GetXSecSumSpline(int tgtpdg, int nupdg);
    std::vector<std::string> GetXSecSplineNames(int tgtpdg, int nupdg);
    genie::Spline const *GetXSecSpline(std::string const &int_name, int tgtpdg,
                                       int nupdg);

    ~XSSplines();
  };

  GHEP3EventSource(YAML::Node const &cfg);

  std::shared_ptr<HepMC3::GenEvent> first();

  std::shared_ptr<HepMC3::GenEvent> next();

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg);

  genie::EventRecord const *EventRecord(HepMC3::GenEvent const &ev);

  virtual ~GHEP3EventSource();

private:
  std::unique_ptr<XSSplines> gsplines;
};

} // namespace nuis
