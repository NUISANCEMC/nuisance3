#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/eventinput/plugins/ROOTUtils.h"

#include "nvconv.h"
#include "nvfatxtools.h"

#include "TChain.h"
#include "TFile.h"

#include "HepMC3/GenRunInfo.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <fstream>

namespace nuis {

NEW_NUISANCE_EXCEPT(NeutVectNoFluxRateHistos);

neutvectEventSource::neutvectEventSource(YAML::Node const &cfg) {
  if (cfg["filepath"] &&
      HasTTree(cfg["filepath"].as<std::string>(), "neuttree")) {
    filepaths.push_back(cfg["filepath"].as<std::string>());
  } else if (cfg["filepaths"]) {
    for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
      if (HasTTree(fp, "neuttree")) {
        filepaths.push_back(fp);
      }
    }
  }
};

std::shared_ptr<HepMC3::GenEvent> neutvectEventSource::first() {

  if (!filepaths.size()) {
    return nullptr;
  }

  chin = std::make_unique<TChain>("neuttree");

  for (auto const &ftr : filepaths) {
    if (!chin->Add(ftr.c_str(), 0)) {
      log_warn("Could not find neuttree in {}", ftr.native());
      chin.reset();
      return nullptr;
    }
  }

  ch_ents = chin->GetEntries();
  ient = 0;

  if (ch_ents == 0) {
    return nullptr;
  }

  nv = nullptr;
  chin->SetAutoDelete(true);
  chin->UseCache(100000);
  chin->SetCacheSize(100000);
  chin->SetBranchStatus("vertexbranch", false);
  chin->SetBranchStatus("vectorbranch", true);
  auto branch_status = chin->SetBranchAddress("vectorbranch", &nv);
  // should check this
  (void)branch_status;
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
      log_critical("Couldn't get nvconv::GetFluxRateHistPairFromChain");
      throw NeutVectNoFluxRateHistos();
    }
  }

  gri = nvconv::BuildRunInfo(ch_ents, fatx, flux_hist, isMonoE, beam_pid,
                             flux_energy_to_MeV);

  ch_fuid = chin->GetFile()->GetUUID();
  ient = 0;
  auto ge = nvconv::ToGenEvent(nv, gri);
  ge->set_event_number(ient);
  ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);
  return ge;
}

std::shared_ptr<HepMC3::GenEvent> neutvectEventSource::next() {
  ient++;

  if (ient >= ch_ents) {
    return nullptr;
  }

  chin->GetEntry(ient);

  if (chin->GetFile()->GetUUID() != ch_fuid) {
    ch_fuid = chin->GetFile()->GetUUID();
  }

  auto ge = nvconv::ToGenEvent(nv, gri);
  ge->set_event_number(ient);
  ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);
  return ge;
}

NeutVect *neutvectEventSource::neutvect(HepMC3::GenEvent const &ev) {
  chin->GetEntry(ev.event_number());
  return nv;
}

IEventSourcePtr neutvectEventSource::MakeEventSource(YAML::Node const &cfg) {
  return std::make_shared<neutvectEventSource>(cfg);
}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::neutvectEventSource::MakeEventSource, MakeEventSource);
#endif

} // namespace nuis
