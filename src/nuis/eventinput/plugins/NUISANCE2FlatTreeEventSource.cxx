#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/eventinput/IEventSource.h"

#include "nuis/eventinput/plugins/ROOTUtils.h"

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/WriterUtils.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"
#include "HepMC3/Print.h"

#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include "yaml-cpp/yaml.h"

#include "boost/dll/alias.hpp"

#include <fstream>

namespace nuis {

std::map<int, std::pair<std::string, std::string>> Modes = {
    {700, {"NuElectronElastic", ""}},
    {701, {"InverseMuDecay", ""}},
    {-200, {"EM QuasiElastic", ""}},
    {-300, {"EM MEC", ""}},
    {-400, {"EM Resonant", ""}},
    {-600, {"EM DeepInelastic", ""}},
    {200, {"CC QuasiElastic", ""}},
    {250, {"NC Elastic", ""}},
    {300, {"CC MEC", ""}},
    {350, {"NC MEC", ""}},
    {400, {"CC Resonant", ""}},
    {450, {"NC Resonant", ""}},
    {500, {"CC ShallowInelastic", ""}},
    {550, {"NC ShallowInelastic", ""}},
    {600, {"CC DeepInelastic", ""}},
    {650, {"NC DeepInelastic", ""}},
    {100, {"CC Coherent", ""}},
    {150, {"NC Coherent", ""}},
    {101, {"CC Diffractive", ""}},
    {151, {"NC Diffractive", ""}},
};

std::map<int, int> NEUTToMode{
    {1, 200},  {2, 300},  {11, 400}, {12, 400}, {13, 400}, {16, 100},
    {21, 500}, {22, 500}, {23, 500}, {26, 600}, {31, 450}, {32, 450},
    {33, 450}, {34, 450}, {36, 150}, {41, 550}, {42, 550}, {43, 550},
    {44, 550}, {45, 550}, {46, 650}, {51, 250}, {52, 250}};

std::shared_ptr<HepMC3::GenRunInfo> BuildRunInfo(Long64_t nevents,
                                                 double fatx) {

  // G.R.1 Valid GenRunInfo
  auto run_info = std::make_shared<HepMC3::GenRunInfo>();

  // G.R.2 NuHepMC Version
  NuHepMC::GR2::WriteVersion(run_info);

  // G.R.3 Generator Identification
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "NUISANCE2", "UNKNOWN_NUISANCE2_VERSION", ""});

  // G.R.4 Process Metadata
  NuHepMC::GR4::WriteProcessIDDefinitions(run_info, Modes);

  // G.R.5 Vertex Status Metadata
  NuHepMC::StatusCodeDescriptors VertexStatuses = {
      {NuHepMC::VertexStatus::Primary,
       {"PrimaryVertex", "The neutrino hard-scatter vertex"}},
      {NuHepMC::VertexStatus::FSISummary,
       {"FSIVertex", "A single vertex representing the cascade"}},
      {NuHepMC::VertexStatus::NucleonSeparation,
       {"NucleonSeparationVertex",
        "Impulse approximation vertex that represents the separation of the "
        "single target nucleon from the target nucleus ground state."}},
  };
  NuHepMC::GR5::WriteVertexStatusIDDefinitions(run_info, VertexStatuses);

  NuHepMC::StatusCodeDescriptors ParticleStatuses = {
      {NuHepMC::ParticleStatus::UndecayedPhysical,
       {"UndecayedPhysical",
        "Physical final state particles produced by this simulation"}},
      {NuHepMC::ParticleStatus::DecayedPhysical,
       {"DecayedPhysical", "Particle was decayed by the simulation"}},
      {NuHepMC::ParticleStatus::DocumentationLine,
       {"DocumentationLine",
        "Documentation line, not considered a real particle"}},
      {NuHepMC::ParticleStatus::IncomingBeam,
       {"IncomingBeam", "Incoming beam particle"}},
      {NuHepMC::ParticleStatus::Target,
       {"TargetParticle", "The target particle in the hard scatter"}},
      {NuHepMC::ParticleStatus::StruckNucleon,
       {"StruckNucleon", "The nucleon involved in the hard scatter"}},
  };
  NuHepMC::GR6::WriteParticleStatusIDDefinitions(run_info, ParticleStatuses);

  // G.R.7 Event Weights
  NuHepMC::GR7::SetWeightNames(run_info, {
                                             "CV",
                                         });

  // G.C.1 Signalling Followed Conventions
  std::vector<std::string> conventions = {
      "G.C.1", "G.C.2", "G.C.4", "G.C.5", "E.C.1", "V.C.1", "P.C.1", "P.C.2",
  };

  // G.C.4 Cross Section Units and Target Scaling
  NuHepMC::GC4::SetCrossSectionUnits(run_info, "1e-38 cm2", "PerTargetNucleon");

  NuHepMC::GC5::SetFluxAveragedTotalXSec(run_info, fatx);

  // G.C.2 File Exposure (Standalone)
  NuHepMC::GC2::SetExposureNEvents(run_info, nevents);

  // G.C.1 Signalling Followed Conventions
  NuHepMC::GC1::SetConventions(run_info, conventions);

  return run_info;
}

class NUISANCE2FlattTreeEventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ient;

  std::unique_ptr<TTreeReader> reader;

  std::unique_ptr<TTreeReaderValue<int>> nfsp;
  std::unique_ptr<TTreeReaderArray<float>> px;
  std::unique_ptr<TTreeReaderArray<float>> py;
  std::unique_ptr<TTreeReaderArray<float>> pz;
  std::unique_ptr<TTreeReaderArray<float>> E;
  std::unique_ptr<TTreeReaderArray<int>> pdg;

  std::unique_ptr<TTreeReaderValue<int>> ninitp;
  std::unique_ptr<TTreeReaderArray<float>> px_init;
  std::unique_ptr<TTreeReaderArray<float>> py_init;
  std::unique_ptr<TTreeReaderArray<float>> pz_init;
  std::unique_ptr<TTreeReaderArray<float>> E_init;
  std::unique_ptr<TTreeReaderArray<int>> pdg_init;

  std::unique_ptr<TTreeReaderValue<int>> nvertp;
  std::unique_ptr<TTreeReaderArray<float>> px_vert;
  std::unique_ptr<TTreeReaderArray<float>> py_vert;
  std::unique_ptr<TTreeReaderArray<float>> pz_vert;
  std::unique_ptr<TTreeReaderArray<float>> E_vert;
  std::unique_ptr<TTreeReaderArray<int>> pdg_vert;

  std::unique_ptr<TTreeReaderValue<double>> fScaleFactor;

  std::unique_ptr<TTreeReaderValue<int>> Mode;

  std::unique_ptr<TTreeReaderValue<int>> PDGnu;
  std::unique_ptr<TTreeReaderValue<int>> tgt;

  std::shared_ptr<HepMC3::GenEvent> ToGenEvent() {
    auto evt = std::make_shared<HepMC3::GenEvent>(HepMC3::Units::GEV);

    auto proc_id = **Mode;
    NuHepMC::ER3::SetProcessID(*evt, proc_id);

    auto primary_vtx = std::make_shared<HepMC3::GenVertex>();
    primary_vtx->set_status(NuHepMC::VertexStatus::Primary);

    auto nucleon_separation_vtx = std::make_shared<HepMC3::GenVertex>();
    nucleon_separation_vtx->set_status(
        NuHepMC::VertexStatus::NucleonSeparation);

    auto fsi_vtx = std::make_shared<HepMC3::GenVertex>();
    fsi_vtx->set_status(NuHepMC::VertexStatus::FSISummary);

    evt->add_vertex(primary_vtx);
    evt->add_vertex(fsi_vtx);

    for (int fs_it = 0; fs_it < **nfsp; ++fs_it) {

      auto pid = pdg->operator[](fs_it);

      auto part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{px->operator[](fs_it), py->operator[](fs_it),
                             pz->operator[](fs_it), E->operator[](fs_it)},
          pid, NuHepMC::ParticleStatus::UndecayedPhysical);

      fsi_vtx->add_particle_out(part);
    }

    std::shared_ptr<HepMC3::GenParticle> tgt_part{nullptr},
        struck_nuc_part{nullptr};

    for (int in_it = 0; in_it < **ninitp; ++in_it) {

      auto pid = pdg_init->operator[](in_it);

      auto part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{
              px_init->operator[](in_it), py_init->operator[](in_it),
              pz_init->operator[](in_it), E_init->operator[](in_it)},
          pid, 0);

      if (pid == **PDGnu) {
        part->set_status(NuHepMC::ParticleStatus::IncomingBeam);
        primary_vtx->add_particle_in(part);
      } else if (pid == **tgt) {
        part->set_status(NuHepMC::ParticleStatus::Target);
        tgt_part = part;
      } else if ((pid == 2212) || (pid == 2112)) {
        part->set_status(NuHepMC::ParticleStatus::StruckNucleon);
        struck_nuc_part = part;
      }
    }

    if (struck_nuc_part && tgt_part) {
      evt->add_vertex(nucleon_separation_vtx);
      nucleon_separation_vtx->add_particle_in(tgt_part);
      nucleon_separation_vtx->add_particle_out(struck_nuc_part);
      primary_vtx->add_particle_in(struck_nuc_part);
    } else if (tgt_part) {
      primary_vtx->add_particle_in(tgt_part);
    }

    // skip ninitp as they are in both array
    for (int vt_it = **ninitp; vt_it < **nvertp; ++vt_it) {

      auto pid = pdg_vert->operator[](vt_it);

      auto part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{
              px_vert->operator[](vt_it), py_vert->operator[](vt_it),
              pz_vert->operator[](vt_it), E_vert->operator[](vt_it)},
          pid, NuHepMC::ParticleStatus::DocumentationLine);

      primary_vtx->add_particle_out(part);
      fsi_vtx->add_particle_in(part);
    }

    auto beamp = NuHepMC::Event::GetBeamParticle(*evt);
    auto tgtp = NuHepMC::Event::GetTargetParticle(*evt);
    auto nfs = NuHepMC::Event::GetParticles_All(
                   *evt, NuHepMC::ParticleStatus::UndecayedPhysical)
                   .size();

    if (!beamp) {
      log_critical("NUISANCE2FlatTree event contained no beam particle");
    }
    if (!tgtp) {
      log_critical("NUISANCE2FlatTree event contained no target particle");
    }
    if (!nfs) {
      log_critical(
          "NUISANCE2FlatTree event contained no final state particles");
    }

    if ((!beamp) || (!tgtp) || (!nfs)) {
      HepMC3::Print::content(*evt);
      abort();
    }

    evt->weights().push_back(1);

    return evt;
  }

public:
  NUISANCE2FlattTreeEventSource(YAML::Node const &cfg) {
    if (cfg["filepath"] &&
        HasTTree(cfg["filepath"].as<std::string>(), "FlatTree_VARS")) {
      filepaths.push_back(cfg["filepath"].as<std::string>());
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
        log_trace("Checking file {} for tree FlatTree_VARS.", fp);
        if (HasTTree(fp, "FlatTree_VARS")) {
          filepaths.push_back(fp);
          log_debug("Added file {} with tree FlatTree_VARS to filepaths.", fp);
        }
      }
    }
  }

  std::shared_ptr<HepMC3::GenEvent> first() {

    if (!filepaths.size()) {
      return nullptr;
    }

    chin = std::make_unique<TChain>("FlatTree_VARS");

    for (auto const &ftr : filepaths) {
      if (!chin->Add(ftr.c_str(), 0)) {
        log_warn("Could not find FlatTree_VARS in {}", ftr.native());
        chin.reset();
        return nullptr;
      } else {
        log_debug("Added file {} with tree FlatTree_VARS to TChain.",
                  ftr.native());
      }
    }

    reader = std::make_unique<TTreeReader>(chin.get());

    if (reader->GetEntries() == 0) {
      return nullptr;
    }

    nfsp = std::make_unique<TTreeReaderValue<int>>(*reader, "nfsp");
    px = std::make_unique<TTreeReaderArray<float>>(*reader, "px");
    py = std::make_unique<TTreeReaderArray<float>>(*reader, "py");
    pz = std::make_unique<TTreeReaderArray<float>>(*reader, "pz");
    E = std::make_unique<TTreeReaderArray<float>>(*reader, "E");
    pdg = std::make_unique<TTreeReaderArray<int>>(*reader, "pdg");

    ninitp = std::make_unique<TTreeReaderValue<int>>(*reader, "ninitp");
    px_init = std::make_unique<TTreeReaderArray<float>>(*reader, "px_init");
    py_init = std::make_unique<TTreeReaderArray<float>>(*reader, "py_init");
    pz_init = std::make_unique<TTreeReaderArray<float>>(*reader, "pz_init");
    E_init = std::make_unique<TTreeReaderArray<float>>(*reader, "E_init");
    pdg_init = std::make_unique<TTreeReaderArray<int>>(*reader, "pdg_init");

    nvertp = std::make_unique<TTreeReaderValue<int>>(*reader, "nvertp");
    px_vert = std::make_unique<TTreeReaderArray<float>>(*reader, "px_vert");
    py_vert = std::make_unique<TTreeReaderArray<float>>(*reader, "py_vert");
    pz_vert = std::make_unique<TTreeReaderArray<float>>(*reader, "pz_vert");
    E_vert = std::make_unique<TTreeReaderArray<float>>(*reader, "E_vert");
    pdg_vert = std::make_unique<TTreeReaderArray<int>>(*reader, "pdg_vert");

    fScaleFactor =
        std::make_unique<TTreeReaderValue<double>>(*reader, "fScaleFactor");

    Mode = std::make_unique<TTreeReaderValue<int>>(*reader, "Mode");

    tgt = std::make_unique<TTreeReaderValue<int>>(*reader, "tgt");
    PDGnu = std::make_unique<TTreeReaderValue<int>>(*reader, "PDGnu");

    ient = 0;

    if (!reader->Next()) {
      return nullptr;
    }

    auto ge = ToGenEvent();

    gri = BuildRunInfo(reader->GetEntries(),
                       *(*fScaleFactor) * double(reader->GetEntries()));

    ge->set_event_number(ient++);
    ge->set_run_info(gri);
    ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);

    return ge;
  }

  std::shared_ptr<HepMC3::GenEvent> next() {

    if (!reader->Next()) {
      return nullptr;
    }

    auto ge = ToGenEvent();

    ge->set_event_number(ient++);
    ge->set_run_info(gri);
    ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);

    return ge;
  }

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg) {
    return std::make_shared<NUISANCE2FlattTreeEventSource>(cfg);
  }
};

BOOST_DLL_ALIAS(nuis::NUISANCE2FlattTreeEventSource::MakeEventSource,
                MakeEventSource);

} // namespace nuis