#include "nuis/eventinput/IEventSource.h"

#include "nuis/eventinput/plugins/ROOTUtils.h"

#include "nuwroconv.h"

#include "HepMC3/GenRunInfo.h"

#include "TChain.h"
#include "TFile.h"

#ifdef USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <fstream>

namespace nuis {

class NuWroevent1EventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  double fatx;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  event *ev;

public:
  NuWroevent1EventSource(YAML::Node const &cfg) {
    if (cfg["filepath"] &&
        HasTTree(cfg["filepath"].as<std::string>(), "treeout")) {
      filepaths.push_back(cfg["filepath"].as<std::string>());
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
        if (HasTTree(fp, "treeout")) {
          filepaths.push_back(fp);
        }
      }
    }
  };

  std::shared_ptr<HepMC3::GenEvent> first() {

    if (!filepaths.size()) {
      return nullptr;
    }

    chin = std::make_unique<TChain>("treeout");

    for (auto const &ftr : filepaths) {
      if (!chin->Add(ftr.c_str(), 0)) {
        log_warn("Could not find treeout in {}", ftr.native());
        chin.reset();
        return nullptr;
      }
    }

    ch_ents = chin->GetEntries();
    ient = 0;

    if (ch_ents == 0) {
      return nullptr;
    }

    ev = nullptr;
    auto branch_status = chin->SetBranchAddress("e", &ev);
    // should check this
    (void)branch_status;
    chin->GetEntry(0);

    fatx = ev->weight;

    gri = nuwroconv::BuildRunInfo(ch_ents, fatx, ev->par);

    ch_fuid = chin->GetFile()->GetUUID();
    ient = 0;
    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge->set_event_number(ient);
    ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);
    return ge;
  }

  std::shared_ptr<HepMC3::GenEvent> next() {
    ient++;

    if (ient >= ch_ents) {
      return nullptr;
    }

    chin->GetEntry(ient);

    if (chin->GetFile()->GetUUID() != ch_fuid) {
      ch_fuid = chin->GetFile()->GetUUID();
    }

    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge->set_event_number(ient);
    ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);
    // accounts for per-file variations over an input chain
    ge->weights()[0] = ev->weight / fatx;
    return ge;
  }

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg) {
    return std::make_shared<NuWroevent1EventSource>(cfg);
  }

  virtual ~NuWroevent1EventSource() {}
};

IEventSourcePtr NuWroevent1EventSource_MakeEventSource(YAML::Node const &cfg) {
  return NuWroevent1EventSource::MakeEventSource(cfg);
}

#ifdef USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::NuWroevent1EventSource::MakeEventSource, MakeEventSource);
#endif

} // namespace nuis
