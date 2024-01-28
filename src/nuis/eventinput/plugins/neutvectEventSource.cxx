#include "nuis/eventinput/IEventSource.h"

#include "nvconv.h"
#include "nvfatxtools.h"

#include "HepMC3/GenRunInfo.h"

#include "TChain.h"
#include "TFile.h"

#include "boost/dll/alias.hpp"

#include "yaml-cpp/yaml.h"

namespace nuis {

class neutvectEventSource : public IEventSource {

  std::vector<string> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  NeutVect *nv;

public:
  neutvectEventSource(YAML::Node const &cfg) {
    if (cfg["filepath"]) {
      filepaths.push_back(cfg["filepath"].as<std::string>());
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
        filepaths.push_back(fp);
      }
    }
  };

  std::optional<HepMC3::GenEvent> first() {

    chin = std::make_unique<TChain>("neuttree");

    for (auto const &ftr : filepaths) {
      if (!chin->Add(ftr.c_str(), 0)) {
        std::cout << "[ERROR]: Failed to find tree: \"neuttree\" in file: \""
                  << ftr << "\"." << std::endl;
        return std::optional<HepMC3::GenEvent>();
      }
    }

    ch_ents = chin->GetEntries();
    nv = nullptr;
    auto branch_status = chin->SetBranchAddress("vectorbranch", &nv);
    chin->GetEntry(0);
    int beam_pid = nv->PartInfo(0)->fPID;
    double flux_energy_to_MeV = 1E3;

    double fatx = 0;

    bool isMonoE = nvconv::isMono(*chin, nv);

    std::unique_ptr<TH1> flux_hist(nullptr);

    if (isMonoE) {
      chin->GetEntry(0);
      fatx = nv->Totcrs * 1E-2;
    } else {

      auto frpair = nvconv::GetFluxRateHistPairFromChain(*chin);
      if (frpair.second) {
        fatx = 1E-2 * (frpair.first->Integral() / frpair.second->Integral());
        flux_hist = std::move(frpair.second);
      } else {
        std::cout << "Couldn't get fluxratehistparifromchain..." << std::endl;
        abort();
      }
    }

    gri = nvconv::BuildRunInfo(0, fatx, flux_hist, isMonoE, beam_pid,
                               flux_energy_to_MeV);

    ch_fuid = chin->GetFile()->GetUUID();
    ient = 0;
    auto ge = nvconv::ToGenEvent(nv, gri);
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

    auto ge = nvconv::ToGenEvent(nv, gri);
    ge.set_event_number(ient);
    return ge;
  }

  static IEventSourcePtr Make_neutvectEventSource(YAML::Node const &cfg) {
    return std::make_shared<neutvectEventSource>(cfg);
  }
};

BOOST_DLL_ALIAS(nuis::neutvectEventSource::Make_neutvectEventSource,
                Make_neutvectEventSource);

} // namespace nuis
