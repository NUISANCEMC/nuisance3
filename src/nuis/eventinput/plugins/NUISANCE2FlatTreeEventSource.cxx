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
#include "TH1D.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include "yaml-cpp/yaml.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <fstream>

namespace nuis {

const std::map<int, std::pair<std::string, int>> ChannelNameIndexModeMapping{
    {1, {"CC_QE_nu", 200}},

    {11, {"CC_RES_ppi+_nu", 400}},
    {12, {"CC_RES_ppi0_nu", 401}},
    {13, {"CC_RES_npi+_nu", 402}},

    {21, {"CC_multi_pi_nu", 500}},

    {31, {"NC_RES_npi0_nu", 450}},
    {32, {"NC_RES_ppi0_nu", 451}},
    {33, {"NC_RES_ppi-_nu", 452}},
    {34, {"NC_RES_npi+_nu", 452}},

    {41, {"NC_multi_pi_nu", 550}},

    {51, {"NC_elastic_p_nu", 250}},
    {52, {"NC_elastic_n_nu", 251}},
    {53, {"NC_2p2h_nu", 350}},

    {16, {"CC_COH_nu", 100}},

    {36, {"NC_COH_nu", 150}},

    {22, {"CC_eta_nu", 410}},

    {42, {"NC_eta_n_nu", 460}},
    {43, {"NC_eta_p_nu", 461}},

    {23, {"CC_kaon_nu", 411}},

    {44, {"NC_kaon_n_nu", 462}},
    {45, {"NC_kaon_p_nu", 463}},

    {26, {"CC_DIS_nu", 600}},

    {46, {"NC_DIS_nu", 601}},

    {17, {"CC_1gamma_nu", 412}},

    {38, {"NC_1gamma_n_nu", 464}},
    {39, {"NC_1gamma_p_nu", 465}},

    {2, {"CC_2p2h_nu", 300}},

    {15, {"CC_DIF_nu", 110}},

    {35, {"NC_DIF_nu", 160}},

    {-1, {"CC_QE_proton_nubar", 225}},

    {-11, {"CC_RES_npi-_nubar", 425}},
    {-12, {"CC_RES_ppi0_nubar", 426}},
    {-13, {"CC_RES_ppi-_nubar", 427}},

    {-21, {"CC_multi_pi_nubar", 525}},

    {-31, {"NC_RES_npi0_nubar", 475}},
    {-32, {"NC_RES_ppi0_nubar", 476}},
    {-33, {"NC_RES_ppi-_nubar", 478}},
    {-34, {"NC_RES_npi+_nubar", 479}},

    {-41, {"NC_multi_pi_nubar", 575}},

    {-51, {"NC_elastic_p_nubar", 275}},
    {-52, {"NC_elastic_n_nubar", 276}},

    {-53, {"NC_2p2h_nubar", 355}},

    {-16, {"CC_COH_nubar", 125}},

    {-36, {"NC_COH_nubar", 175}},

    {-22, {"CC_eta_nubar", 435}},

    {-42, {"NC_eta_n_nubar", 485}},
    {-43, {"NC_eta_p_nubar", 486}},

    {-23, {"CC_kaon_nubar", 436}},

    {-44, {"NC_kaon_n_nubar", 487}},
    {-45, {"NC_kaon_p_nubar", 488}},

    {-26, {"CC_DIS_nubar", 625}},

    {-46, {"NC_DIS_nubar", 675}},

    {-17, {"CC_1gamma_nubar", 437}},

    {-38, {"NC_1gamma_n_nubar", 489}},
    {-39, {"NC_1gamma_p_nubar", 490}},

    {-2, {"CC_2p2h_nubar", 325}},

    {-15, {"CC_DIF_nubar", 135}},

    {-35, {"NC_DIF_nubar", 185}},

    {54, {"NuElectronElastic", 700}},

    {55, {"InverseMuDecay", 701}},
};

int GetEC1Channel(int neutmode) {
  if (!ChannelNameIndexModeMapping.count(neutmode)) {
    std::cout << "[ERROR]: neutmode: " << neutmode << " unaccounted for."
              << std::endl;
    throw neutmode;
  }
  return ChannelNameIndexModeMapping.at(neutmode).second;
}

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(Long64_t nevents, double fatx, int probepid,
             std::unique_ptr<TH1D> const &flux_hist) {

  // G.R.1 Valid GenRunInfo
  auto run_info = std::make_shared<HepMC3::GenRunInfo>();

  // G.R.2 NuHepMC Version
  NuHepMC::GR2::WriteVersion(run_info);

  // G.R.3 Generator Identification
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "NUISANCE2", "UNKNOWN_NUISANCE2_VERSION", ""});

  // G.R.4 Process Metadata
  std::vector<int> pids;
  for (auto const &neutchan : ChannelNameIndexModeMapping) {
    pids.push_back(neutchan.second.second);
    NuHepMC::add_attribute(run_info,
                           "NuHepMC.ProcessInfo[" +
                               std::to_string(neutchan.second.second) +
                               "].Name",
                           neutchan.second.first);
    NuHepMC::add_attribute(
        run_info,
        "NuHepMC.ProcessInfo[" + std::to_string(neutchan.second.second) +
            "].Description",
        std::string("neutmode=") + std::to_string(neutchan.first));
  }

  NuHepMC::add_attribute(run_info, "NuHepMC.ProcessIDs", pids);

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

  if (flux_hist) {
    conventions.push_back("G.C.7");

    NuHepMC::GC7::SetHistogramBeamType(run_info);

    std::vector<double> bin_edges;
    std::vector<double> bin_content;

    bin_edges.push_back(flux_hist->GetXaxis()->GetBinLowEdge(1));
    for (int i = 0; i < flux_hist->GetXaxis()->GetNbins(); ++i) {
      bin_edges.push_back(flux_hist->GetXaxis()->GetBinUpEdge(i + 1));
      bin_content.push_back(flux_hist->GetBinContent(i + 1));
    }

    NuHepMC::GC7::WriteBeamEnergyHistogram(run_info, probepid, bin_edges,
                                           bin_content, false);
    NuHepMC::GC7::WriteBeamUnits(
        run_info, "Unknown", std::string(flux_hist->GetYaxis()->GetTitle()));
  }

  // G.C.4 Cross Section Units and Target Scaling
  NuHepMC::GC4::SetCrossSectionUnits(run_info, "1e-38 cm2", "PerTargetNucleon");

  NuHepMC::GC5::SetFluxAveragedTotalXSec(run_info, fatx);

  // G.C.2 File Exposure (Standalone)
  NuHepMC::GC2::SetExposureNEvents(run_info, nevents);

  // G.C.1 Signalling Followed Conventions
  NuHepMC::GC1::SetConventions(run_info, conventions);

  return run_info;
}

class NUISANCE2FlatTreeEventSource : public IEventSource {

  std::vector<std::filesystem::path> filepaths;
  std::unique_ptr<TChain> chin;

  std::shared_ptr<HepMC3::GenRunInfo> gri;

  Long64_t ient;
  double fatx;

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

    NuHepMC::ER3::SetProcessID(*evt, GetEC1Channel(**Mode));

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
      } else { // looking for struck nucleon/target nucleus
        // if the tgt is a nuclear code for hydrogen
        if (**tgt == 1000010010) { // hydrogen/proton
          if ((pid == 2212) || (pid == **tgt)) {
            part->set_status(NuHepMC::ParticleStatus::Target);
            tgt_part = part;
          }
        } else if (**tgt == 1000000010) { // free nucleon
          if ((pid == 2112) || (pid == **tgt)) {
            part->set_status(NuHepMC::ParticleStatus::Target);
            tgt_part = part;
          }
          // otherwise assume we have a bigger nucleus and can build the target
          // nuclear particle and maybe struck nucleon
        } else {
          if (pid == **tgt) {
            part->set_status(NuHepMC::ParticleStatus::Target);
            tgt_part = part;
          } else if ((pid == 2212) || (pid == 2112)) {
            part->set_status(NuHepMC::ParticleStatus::StruckNucleon);
            struck_nuc_part = part;
          }
        }
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
      log_debug("NUISANCE2FlatTree event contained no beam particle, giving a "
                "weight of 0.");
    }
    if (!tgtp) {
      if (**tgt > 1000000000) {
        // hack in a dummy nuclear particle because I can't even
        primary_vtx->add_particle_in(std::make_shared<HepMC3::GenParticle>(
            HepMC3::FourVector{}, **tgt, NuHepMC::ParticleStatus::Target));

        tgtp = NuHepMC::Event::GetTargetParticle(*evt);
      } else {
        log_debug("NUISANCE2FlatTree event contained no target particle, "
                  "giving a weight of 0.");
      }
    }
    if (!nfs) {
      log_debug("NUISANCE2FlatTree event contained no final state particles, "
                "giving a weight of 0.");
    }

    if ((!beamp) || (!tgtp) || (!nfs)) {
      evt->weights().push_back(0);
    } else {
      evt->weights().push_back(1);
    }

    NuHepMC::ER5::SetLabPosition(*evt, std::vector<double>{0, 0, 0, 0});

    return evt;
  }

public:
  NUISANCE2FlatTreeEventSource(YAML::Node const &cfg) {
    log_trace("[NUISANCE2FlatTreeEventSource] enter");
    if (cfg["filepath"]) {
      log_trace("Checking file {} for tree FlatTree_VARS.",
                cfg["filepath"].as<std::string>());
      if (HasTTree(cfg["filepath"].as<std::string>(), "FlatTree_VARS")) {
        log_debug("Added file {} with tree FlatTree_VARS to filepaths.",
                  cfg["filepath"].as<std::string>());
        filepaths.push_back(cfg["filepath"].as<std::string>());
      }
    } else if (cfg["filepaths"]) {
      for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
        log_trace("Checking file {} for tree FlatTree_VARS.", fp);
        if (HasTTree(fp, "FlatTree_VARS")) {
          filepaths.push_back(fp);
          log_debug("Added file {} with tree FlatTree_VARS to filepaths.", fp);
        }
      }
    }
    log_trace("[NUISANCE2FlatTreeEventSource] exit");
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

    std::unique_ptr<TH1D> flux(
        reader->GetTree()->GetCurrentFile()->Get<TH1D>("FlatTree_FLUX"));
    if (flux) {
      flux->SetDirectory(nullptr);
    }

    fatx = *(*fScaleFactor) * double(reader->GetEntries()) * 1E38;
    gri = BuildRunInfo(reader->GetEntries(), fatx,
                       NuHepMC::Event::GetBeamParticle(*ge)->pid(), flux);

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
    // accounts for per-file variations over an input chain
    ge->weights()[0] *=
        *(*fScaleFactor) * double(reader->GetEntries()) * 1E38 / fatx;

    return ge;
  }

  static IEventSourcePtr MakeEventSource(YAML::Node const &cfg) {
    return std::make_shared<NUISANCE2FlatTreeEventSource>(cfg);
  }
};

IEventSourcePtr
NUISANCE2FlatTreeEventSource_MakeEventSource(YAML::Node const &cfg) {
  return NUISANCE2FlatTreeEventSource::MakeEventSource(cfg);
}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::NUISANCE2FlatTreeEventSource::MakeEventSource,
                MakeEventSource);
#endif

} // namespace nuis