#include "nuis/eventinput/IEventSource.h"

#include "neutvect.h"

#include "TChain.h"
#include "TFile.h"

#include "boost/dll/alias.hpp"

#include "yaml-cpp/yaml.h"

namespace nuis {

class neutvectEventSource : public IEventSource {

  std::vector<string> filepaths;
  std::unique_ptr<TChain> chin;

  Long64_t ch_ents;
  Long64_t ient;
  TUUID ch_fuid;

  NeutVect *nv;

public:
  neutvectEventSource(YAML::Node const &cfg) {
    if (cfg["filepath"]) {
      filepaths.push_back(cfg["filepath"].as<std::string>());
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepath"].as<std::vector<std::string>>()) {
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

    // auto gri = BuildRunInfo(ents_to_run, fatx, flux_histo, isMonoE, beam_pid,
    //                         flux_energy_to_MeV);
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

    //   auto hepev = ToGenEvent(nv, gri);

    //   hepev.set_event_number(i);
  }

  static std::unique_ptr<neutvectEventSource>
  Make_neutvectEventSource(YAML::Node const &cfg) {
    return std::make_unique<neutvectEventSource>(cfg);
  }
};

BOOST_DLL_ALIAS(nuis::neutvectEventSource::Make_neutvectEventSource,
                Make_neutvectEventSource);

} // namespace nuis
