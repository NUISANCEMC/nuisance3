#include "nuis/eventinput/IEventSource.h"

#include "nuis/eventinput/plugins/ROOTUtils.h"

#include "nuwroconv.h"

#include "HepMC3/GenRunInfo.h"

#include "TChain.h"
#include "TFile.h"

#include "boost/dll/alias.hpp"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <fstream>

namespace nuis {

class NuWroevent1EventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

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

  std::optional<HepMC3::GenEvent> first() {

    if (!filepaths.size()) {
      return std::optional<HepMC3::GenEvent>();
    }

    chin = std::make_unique<TChain>("treeout");

    for (auto const &ftr : filepaths) {
      if (!chin->Add(ftr.c_str(), 0)) {
        spdlog::warn("Could not find treeout in {}", ftr.native());
        chin.reset();
        return std::optional<HepMC3::GenEvent>();
      }
    }

    ch_ents = chin->GetEntries();
    ient = 0;

    if (ch_ents == 0) {
      return std::optional<HepMC3::GenEvent>();
    }

    ev = nullptr;
    auto branch_status = chin->SetBranchAddress("e", &ev);
    // should check this
    (void)branch_status;
    chin->GetEntry(0);

    double fatx = ev->weight;

    gri = nuwroconv::BuildRunInfo(ch_ents, fatx, ev->par);

    ch_fuid = chin->GetFile()->GetUUID();
    ient = 0;
    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge.set_event_number(ient);
    return ge;
  }

  std::optional<HepMC3::GenEvent> next() {
    ient++;

    if (ient >= ch_ents) {
      return std::optional<HepMC3::GenEvent>();
    }

    chin->GetEntry(ient);

    if (chin->GetFile()->GetUUID() != ch_fuid) {
      ch_fuid = chin->GetFile()->GetUUID();
    }

    auto ge = nuwroconv::ToGenEvent(*ev, gri);
    ge.set_event_number(ient);
    return ge;
  }

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg) {
    return std::make_shared<NuWroevent1EventSource>(cfg);
  }

  virtual ~NuWroevent1EventSource() {}
};

BOOST_DLL_ALIAS(nuis::NuWroevent1EventSource::MakeEventSource, MakeEventSource);

} // namespace nuis
