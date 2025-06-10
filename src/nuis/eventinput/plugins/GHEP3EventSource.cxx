#include "nuis/eventinput/plugins/GHEP3EventSource.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#include "nuis/eventinput/plugins/ROOTUtils.h"

#include "Framework/Conventions/Units.h"
#include "Framework/EventGen/EventRecord.h"
#include "Framework/EventGen/GEVGDriver.h"
#include "Framework/GHEP/GHepParticle.h"
#include "Framework/GHEP/GHepRecord.h"
#include "Framework/GHEP/GHepUtils.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Ntuple/NtpMCEventRecord.h"
#include "Framework/Numerical/Spline.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/Utils/RunOpt.h"
#include "Framework/Utils/XSecSplineList.h"

#include "ProSelecta/unit.h"

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/WriterUtils.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"
#include "HepMC3/Print.h"

#include "TChain.h"
#include "TFile.h"
#include "TGraph.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include <fstream>

namespace nuis {

namespace ghepconv {

DECLARE_NUISANCE_EXCEPT(FailedGHEPParsing);

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

std::shared_ptr<HepMC3::GenRunInfo> BuildRunInfo(bool HaveTotXSSpline) {

  // G.R.1 Valid GenRunInfo
  auto run_info = std::make_shared<HepMC3::GenRunInfo>();

  // G.R.2 NuHepMC Version
  NuHepMC::GR2::WriteVersion(run_info);

  // G.R.3 Generator Identification
  run_info->tools().emplace_back(
      HepMC3::GenRunInfo::ToolInfo{"GENIE", GENIE_VERSION_STR, ""});

  // G.R.8 Process Metadata
  NuHepMC::GR8::WriteProcessIDDefinitions(run_info, Modes);

  // G.R.9 Vertex Status Metadata
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

  NuHepMC::GR9::WriteVertexStatusIDDefinitions(run_info, VertexStatuses);

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
  NuHepMC::GR10::WriteParticleStatusIDDefinitions(run_info, ParticleStatuses);
  // G.R.7 Event Weights
  NuHepMC::GR7::SetWeightNames(run_info, {
                                             "CV",
                                         });
  // G.R.4 Signalling Followed Conventions
  std::vector<std::string> conventions = {
      "E.C.1",
      "V.C.1",
      "P.C.1",
      "P.C.2",
  };

  if (HaveTotXSSpline) {
    conventions.push_back("E.C.2");
    // G.R.6 Cross Section Units and Target Scaling
    NuHepMC::GR6::SetCrossSectionUnits(run_info, "pb", "PerAtom");
  }

  // G.R.4 Signalling Followed Conventions
  NuHepMC::GR4::SetConventions(run_info, conventions);

  return run_info;
}

bool IsPrimaryParticle(::genie::GHepParticle const &p,
                       ::genie::GHepRecord const &GHep) {

  // If initial state nucleon, or nucleon target, it is definintely primary!
  if (p.Status() == genie::kIStInitialState ||
      p.Status() == genie::kIStNucleonTarget)
    return true;

  // Reject intermediate resonant state or pre-DIS state
  if (p.Status() ==
          genie::kIStDISPreFragmHadronicState || // DIS before fragmentation
      p.Status() ==
          genie::kIStPreDecayResonantState ||       // pre decay resonance state
      p.Status() == genie::kIStIntermediateState || // intermediate state
      p.Status() == genie::kIStDecayedState ||      // Decayed state
      p.Status() == genie::kIStUndefined)
    return false; // undefined state

  // Check if the mother is the neutrino or IS nucleon
  if (p.FirstMother() < 2) {
    return true;
  }

  // We've now filtered off intermediate states and obvious initial states

  // Loop over this particle's mothers, grandmothers, and so on cleaning out
  // intermediate particles Set the starting particle to be our particle
  ::genie::GHepParticle *mother = GHep.Particle(p.FirstMother());
  while (mother->FirstMother() > 1) {
    // It could be that the mother's status is actually a decayed state linked
    // back to the vertex
    if (mother->Status() == genie::kIStDecayedState || // A decayed state
        mother->Status() ==
            genie::kIStDISPreFragmHadronicState || // A DIS state before
                                                   // fragmentation
        mother->Status() ==
            genie::kIStPreDecayResonantState) { // A pre-decay resonant state
      mother = GHep.Particle(mother->FirstMother());
    } else { // If not, move out of the loop
      break;
    }
  }

  // Finally, this should mean that our partcile is marked for transport through
  // the nucleus Could also be interactions of free proton
  if (p.Status() ==
          genie::kIStHadronInTheNucleus || // Then require the particle to be
                                           // paseed to FSI
      (p.Status() == genie::kIStStableFinalState && // Can also have interaction
                                                    // on free proton
       GHep.Summary()->InitState().TgtPtr()->A() == 1 &&
       GHep.Summary()->InitState().TgtPtr()->Z() == 1)) {
    return true;
  }

  return false;
}

int GetGENIEParticleStatus(::genie::GHepParticle const &p, int proc_id,
                           int TargetPDG, bool IsFree) {
  /*
     kIStUndefined                  = -1,
     kIStInitialState               =  0,   / generator-level initial state /
     kIStStableFinalState           =  1,   / generator-level final state:
     particles to be tracked by detector-level MC /
     kIStIntermediateState          =  2,
     kIStDecayedState               =  3,
     kIStCorrelatedNucleon          = 10,
     kIStNucleonTarget              = 11,
     kIStDISPreFragmHadronicState   = 12,
     kIStPreDecayResonantState      = 13,
     kIStHadronInTheNucleus         = 14,   / hadrons inside the nucleus:
     marked for hadron transport modules to act on /
     kIStFinalStateNuclearRemnant   = 15,   / low energy nuclear fragments
     entering the record collectively as a 'hadronic blob' pseudo-particle /
     kIStNucleonClusterTarget       = 16,   // for composite nucleons before
     phase space decay
     */

  int state = 0;
  switch (p.Status()) {
  case ::genie::kIStInitialState:
    if ((proc_id == 700) && (p.Pdg() == 11)) {
      state = ::NuHepMC::ParticleStatus::Target;
    } else {
      state = ::NuHepMC::ParticleStatus::IncomingBeam;
    }
    break;
  case ::genie::kIStNucleonTarget:
  case ::genie::kIStCorrelatedNucleon:
  case ::genie::kIStNucleonClusterTarget:
    state = ::NuHepMC::ParticleStatus::StruckNucleon;
    break;

  case ::genie::kIStStableFinalState:
    state = ::NuHepMC::ParticleStatus::UndecayedPhysical;
    break;

  case ::genie::kIStHadronInTheNucleus:
    if (abs(proc_id) == 300)
      state = ::NuHepMC::ParticleStatus::StruckNucleon;
    else
      state = ::NuHepMC::ParticleStatus::DocumentationLine;
    break;

  case ::genie::kIStPreDecayResonantState:
  case ::genie::kIStDISPreFragmHadronicState:
  case ::genie::kIStIntermediateState:
    state = ::NuHepMC::ParticleStatus::DocumentationLine;
    break;

  case ::genie::kIStFinalStateNuclearRemnant:
  case ::genie::kIStUndefined:
  case ::genie::kIStDecayedState:
  default:
    break;
  }

  if (p.Pdg() > 1000000000) {
    if (state ==
        ::NuHepMC::ParticleStatus::IncomingBeam) { // Check that it isn't
                                                   // coherent! Want to
      // keep initial state if so
      state = ::NuHepMC::ParticleStatus::Target;
    } else if (state ==
               ::NuHepMC::ParticleStatus::DocumentationLine) { // nuclear
                                                               // remnant
      state = ::NuHepMC::ParticleStatus::UndecayedPhysical;
    }
  }

  // slight hack for target pdgs being single particle codes for free targets.
  if ((state == ::NuHepMC::ParticleStatus::IncomingBeam) &&
      (p.Pdg() == TargetPDG) && IsFree) {
    state = ::NuHepMC::ParticleStatus::Target;
  }

  return state;
}

int ConvertGENIEReactionCode(::genie::GHepRecord const &GHep) {
  if (GHep.Summary()->ProcInfo().IsNuElectronElastic()) {
    return 700;
  }

  if (GHep.Summary()->ProcInfo().IsIMDAnnihilation() ||
      GHep.Summary()->ProcInfo().IsInverseMuDecay()) {
    return 701;
  }

  // Electron Scattering
  if (GHep.Summary()->ProcInfo().IsEM()) {
    if (GHep.Summary()->InitState().ProbePdg() == 11) {
      if (GHep.Summary()->ProcInfo().IsQuasiElastic())
        return -200;
      else if (GHep.Summary()->ProcInfo().IsMEC())
        return -300;
      else if (GHep.Summary()->ProcInfo().IsResonant())
        return -400;
      else if (GHep.Summary()->ProcInfo().IsDeepInelastic())
        return -600;
      else {
        log_warn(R"(Unknown GENIE Electron Scattering Mode!
  ScatteringTypeId = {}, InteractionTypeId = {}
  {}
  {}
  {})",
                 int(GHep.Summary()->ProcInfo().ScatteringTypeId()),
                 int(GHep.Summary()->ProcInfo().InteractionTypeId()),
                 genie::ScatteringType::AsString(
                     GHep.Summary()->ProcInfo().ScatteringTypeId()),
                 genie::InteractionType::AsString(
                     GHep.Summary()->ProcInfo().InteractionTypeId()),
                 GHep.Summary()->ProcInfo().IsMEC());

        return 0;
      }
    }

    // Weak CC
  } else if (GHep.Summary()->ProcInfo().IsWeakCC()) {
    // CC MEC
    if (GHep.Summary()->ProcInfo().IsMEC()) {
      return 300;
    } else if (GHep.Summary()->ProcInfo().IsDiffractive()) {
      return 101;
      // CC OTHER
    } else {
      int neut_code = ::genie::utils::ghep::NeutReactionCode(&GHep);
      int mode = NEUTToMode[neut_code];
      if (!mode) {
        log_debug("Untranslated NEUT code: {}", neut_code);
      }
      return mode;
    }

    // Weak NC
  } else if (GHep.Summary()->ProcInfo().IsWeakNC()) {
    // NC MEC
    if (GHep.Summary()->ProcInfo().IsMEC()) {
      return 350;
    } else if (GHep.Summary()->ProcInfo().IsDiffractive()) {
      return 151;
      // NC OTHER
    } else {
      int neut_code = ::genie::utils::ghep::NeutReactionCode(&GHep);
      int mode = NEUTToMode[neut_code];
      if (!mode) {
        log_debug("Untranslated NEUT code: {}", neut_code);
      }
      return mode;
    }
  }

  return 0;
}

namespace NuHepMC {
namespace ParticleStatus {
std::string to_string(int ps) {
  switch (ps) {
  case ::NuHepMC::ParticleStatus::UndecayedPhysical:
    return "UndecayedPhysical";
  case ::NuHepMC::ParticleStatus::DecayedPhysical:
    return "DecayedPhysical";
  case ::NuHepMC::ParticleStatus::DocumentationLine:
    return "DocumentationLine";
  case ::NuHepMC::ParticleStatus::IncomingBeam:
    return "IncomingBeam";
  case ::NuHepMC::ParticleStatus::Target:
    return "Target";
  case ::NuHepMC::ParticleStatus::StruckNucleon:
    return "StruckNucleon";
  default:
    return std::string("custom-particle-state-") + std::to_string(ps);
  }
}
} // namespace ParticleStatus
} // namespace NuHepMC

std::string PartToStr(HepMC3::ConstGenParticlePtr pt) {
  if (!pt) {
    return "PARTICLE-NOTFOUND";
  }
  std::stringstream ss;

  std::string status = NuHepMC::ParticleStatus::to_string(pt->status());

  auto mom = pt->momentum();

  ss << "{ id: " << pt->id() << ", pid: " << pt->pid() << ", status: " << status
     << ", p: ( " << mom.x() << ", " << mom.y() << ", " << mom.z()
     << ", E: " << mom.e() << ") GeV }";

  return ss.str();
}

std::string GHEPToStr(::genie::GHepRecord const &GHep) {
  std::stringstream ss;

  ss << "=====================================\n";
  ss << "GHEP\n";
  ss << "  ScatteringTypeId: "
     << int(GHep.Summary()->ProcInfo().ScatteringTypeId()) << ", "
     << genie::ScatteringType::AsString(
            GHep.Summary()->ProcInfo().ScatteringTypeId())
     << "\n";
  ss << "  InteractionTypeId: "
     << int(GHep.Summary()->ProcInfo().InteractionTypeId()) << ", "
     << genie::InteractionType::AsString(
            GHep.Summary()->ProcInfo().InteractionTypeId())
     << "\n\n";

  ss << "  IsNuElectronElastic: "
     << GHep.Summary()->ProcInfo().IsNuElectronElastic() << "\n";
  ss << "  IsWeakCC: " << GHep.Summary()->ProcInfo().IsWeakCC() << "\n";
  ss << "  IsWeakNC: " << GHep.Summary()->ProcInfo().IsWeakNC() << "\n";
  ss << "  IsIMDAnnihilation: "
     << GHep.Summary()->ProcInfo().IsIMDAnnihilation() << "\n";
  ss << "  IsInverseMuDecay: " << GHep.Summary()->ProcInfo().IsInverseMuDecay()
     << "\n";
  ss << "  IsEM: " << GHep.Summary()->ProcInfo().IsEM() << "\n\n";

  ss << "  IsQuasiElastic: " << GHep.Summary()->ProcInfo().IsQuasiElastic()
     << "\n";
  ss << "  IsMEC: " << GHep.Summary()->ProcInfo().IsMEC() << "\n";
  ss << "  IsResonant: " << GHep.Summary()->ProcInfo().IsResonant() << "\n";
  ss << "  IsDeepInelastic: " << GHep.Summary()->ProcInfo().IsDeepInelastic()
     << "\n";
  ss << "  IsDiffractive: " << GHep.Summary()->ProcInfo().IsDiffractive()
     << "\n\n";
  ss << "  NeutReactionCode: " << ::genie::utils::ghep::NeutReactionCode(&GHep)
     << "\n\n";
  ss << "  TargetNucleus: "
     << (GHep.TargetNucleus() ? std::to_string(GHep.TargetNucleus()->Pdg())
                              : std::string("nullptr"))
     << "\n";

  ss << "-------------------------------------\n";
  size_t i = 0;
  for (auto const &po : GHep) {
    ::genie::GHepParticle const &p =
        dynamic_cast<::genie::GHepParticle const &>(*po);
    ss << "  p_" << (i++) << ": " << p.Pdg() << ", " << p.Status() << "\n";
  }
  ss << "=====================================\n";
  return ss.str();
}

std::shared_ptr<HepMC3::GenEvent> ToGenEvent(genie::GHepRecord const &GHep) {
  auto evt = std::make_shared<HepMC3::GenEvent>(HepMC3::Units::GEV);

  auto proc_id = ConvertGENIEReactionCode(GHep);
  bool IsNuElectronElastic = (proc_id == 700);
  ::NuHepMC::ER3::SetProcessID(*evt, proc_id);

  ::NuHepMC::add_attribute(*evt, "GENIE.Resonance",
                           int(GHep.Summary()->ExclTagPtr()->Resonance()));

  int TargetPDG;
  bool IsFree;

  // Set the TargetPDG
  if (GHep.TargetNucleus() != NULL) {
    TargetPDG = GHep.TargetNucleus()->Pdg();
    IsFree = false;
    // Sometimes GENIE scatters off free nucleons, electrons, photons
    // In which TargetNucleus is NULL and we need to find the initial state
    // particle
  } else {
    // Check the particle is an initial state particle
    // Follows GHepRecord::TargetNucleusPosition but doesn't do check on
    // pdg::IsIon
    ::genie::GHepParticle const &p = *GHep.Particle(1);

    // If not an ion but is an initial state particle
    if (!::genie::pdg::IsIon(p.Pdg()) &&
        (p.Status() == ::genie::kIStInitialState)) {
      IsFree = true;
      TargetPDG = p.Pdg();
      // Catch if something strange happens:
      // Here particle 1 is not an initial state particle OR
      // particle 1 is an ion OR
      // both
    } else {
      if (::genie::pdg::IsIon(p.Pdg())) {
        log_critical(
            "Particle 1 in GHepRecord stack is an ion but isn't an initial "
            "state particle");
      } else {
        log_critical(
            "Particle 1 in GHepRecord stack is not an ion but is an initial "
            "state particle");
      }
      throw FailedGHEPParsing();
    }
  }

  auto primary_vtx = std::make_shared<HepMC3::GenVertex>();
  primary_vtx->set_status(::NuHepMC::VertexStatus::Primary);

  auto fsi_vtx = std::make_shared<HepMC3::GenVertex>();
  fsi_vtx->set_status(::NuHepMC::VertexStatus::FSISummary);

  auto nucsep_vtx = std::make_shared<HepMC3::GenVertex>();
  nucsep_vtx->set_status(::NuHepMC::VertexStatus::NucleonSeparation);

  evt->add_vertex(primary_vtx);

  if (!IsNuElectronElastic) { // no FSI for NuEEl
    evt->add_vertex(fsi_vtx);
  }

  // Loop over all particles
  for (auto const &po : GHep) {
    ::genie::GHepParticle const &p =
        dynamic_cast<::genie::GHepParticle const &>(*po);

    // Get Status
    int state = GetGENIEParticleStatus(p, proc_id, TargetPDG, IsFree);

    // Remove Undefined
    if (state == 0) {
      continue;
    }

    auto pid = p.Pdg();

    auto part = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector{p.Px(), p.Py(), p.Pz(), p.E()}, pid, state);

    if (IsPrimaryParticle(p, GHep)) {
      if ((state == ::NuHepMC::ParticleStatus::IncomingBeam)) {
        primary_vtx->add_particle_in(part);
        continue;
      } else if ((state == ::NuHepMC::ParticleStatus::DocumentationLine)) {
        primary_vtx->add_particle_out(part);
        fsi_vtx->add_particle_in(part);
        continue;
      } else if ((state == ::NuHepMC::ParticleStatus::UndecayedPhysical)) {
        if (!IsNuElectronElastic || (pid < 1000000000)) {
          primary_vtx->add_particle_out(part);
        }
        continue;
      } else if ((state == ::NuHepMC::ParticleStatus::Target)) {
        if (IsNuElectronElastic && (pid == 11)) {
          primary_vtx->add_particle_in(part);
        } else {
          nucsep_vtx->add_particle_in(part);
        }
        continue;
      } else if (state == ::NuHepMC::ParticleStatus::StruckNucleon) {
        nucsep_vtx->add_particle_out(part);
        continue;
      }
      log_warn("GHEP3EventSource missed Primary particle: {}", PartToStr(part));
    } else {
      if (pid > 1000000000) { // nuclear remnant
        nucsep_vtx->add_particle_out(part);
        continue;
      } else if ((state == ::NuHepMC::ParticleStatus::UndecayedPhysical)) {
        (IsNuElectronElastic ? primary_vtx : fsi_vtx)->add_particle_out(part);
        continue;
      }
    }
    if ((state == ::NuHepMC::ParticleStatus::UndecayedPhysical)) {
      log_warn("GHEP3EventSource missed Physical particle: {}",
               PartToStr(part));
    }
  }

  if (!IsNuElectronElastic && nucsep_vtx->particles_in().size()) {
    evt->add_vertex(nucsep_vtx);
  }

  auto beamp = ::NuHepMC::Event::GetBeamParticle(*evt);
  auto tgtp = ::NuHepMC::Event::GetTargetParticle(*evt);
  auto nfs = ::NuHepMC::Event::GetParticles_All(
                 *evt, ::NuHepMC::ParticleStatus::UndecayedPhysical)
                 .size();

  if (!beamp) {
    log_critical("GHEP3 event contained no beam particle");
  }
  if (!tgtp) {
    log_critical("GHEP3 event contained no target particle");
  }
  if (!nfs) {
    log_critical("GHEP3 event contained no final state particles");
  }

  if ((!beamp) || (!tgtp) || (!nfs)) {

    for (auto const &po : GHep) {
      ::genie::GHepParticle const &p =
          dynamic_cast<::genie::GHepParticle const &>(*po);

      int state = GetGENIEParticleStatus(p, proc_id, TargetPDG, IsFree);
      auto pid = p.Pdg();

      auto part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{p.Px(), p.Py(), p.Pz(), p.E()}, pid, state);
      log_critical("GHEP3EventSource missed Physical particle: {}",
                   PartToStr(part));
    }

    throw FailedGHEPParsing() << GHEPToStr(GHep) << "\n\n"
                              << "  proc_id = " << proc_id << "\n\n"
                              << GHep;
  }

  if (!IsNuElectronElastic && (tgtp->pid() != TargetPDG)) {
    log_warn("GHEP3EventSource target particle with pid={}, but NUISANCE "
             "target resolver found TargetPDG={}, IsFree={}.",
             tgtp->pid(), TargetPDG, IsFree);
  }

  evt->weights().push_back(1);

  return evt;
}
} // namespace ghepconv

genie::Spline const *GHEP3EventSource::GetSpline(int tgtpdg, int nupdg) {
  if (!EventGeneratorListName.size()) {
    return nullptr;
  }

  tgtpdg = (tgtpdg == 2212) ? 1000010010 : tgtpdg;

  if (!EvGens.count(tgtpdg) || !EvGens[tgtpdg].count(nupdg)) {
    EvGens[tgtpdg][nupdg] = std::make_unique<genie::GEVGDriver>();
    EvGens[tgtpdg][nupdg]->SetEventGeneratorList(EventGeneratorListName);
    EvGens[tgtpdg][nupdg]->Configure(genie::InitialState(tgtpdg, nupdg));
    EvGens[tgtpdg][nupdg]->CreateSplines();
    EvGens[tgtpdg][nupdg]->CreateXSecSumSpline(100, 0.05, 100);
    log_debug(
        "Built XSecSumSpline for nu:{} on tgt:{} with EventGeneratorList: {}",
        nupdg, tgtpdg, EventGeneratorListName);
  }

  return EvGens[tgtpdg][nupdg]->XSecSumSpline();
}

GHEP3EventSource::GHEP3EventSource(YAML::Node const &cfg) {
  log_trace("[GHEP3EventSource] enter");
  if (cfg["filepath"]) {
    log_trace("Checking file {} for tree gtree.",
              cfg["filepath"].as<std::string>());
    if (HasTTree(cfg["filepath"].as<std::string>(), "gtree")) {
      log_debug("Added file {} with tree gtree to filepaths.",
                cfg["filepath"].as<std::string>());
      filepaths.push_back(cfg["filepath"].as<std::string>());
    }
  } else if (cfg["filepaths"]) {
    for (auto fp : cfg["filepaths"].as<std::vector<std::string>>()) {
      log_trace("Checking file {} for tree gtree.", fp);
      if (HasTTree(fp, "gtree")) {
        filepaths.push_back(fp);
        log_debug("Added file {} with tree gtree to filepaths.", fp);
      }
    }
    log_trace("[GHEP3EventSource] exit");
  }

  auto evt = first();
  if (!evt) { // if we can't read an event, there's no point going further
    return;
  }

  genie::Messenger::Instance()->SetPriorityLevel("GHepUtils", pFATAL);

  genie::XSecSplineList *splist = genie::XSecSplineList::Instance();

  std::string SplineXML;
  if (cfg["spline_file"]) {
    SplineXML = cfg["spline_file"].as<std::string>();
  } else if (std::getenv("GENIE_XSEC_FILE")) {
    SplineXML = std::getenv("GENIE_XSEC_FILE");
  }

  if (!SplineXML.size()) {
    log_warn(
        "No GENIE Spline file set. Add \"spline_file\" key to configuration "
        "YAML node or set GENIE_XSEC_FILE in the environment.");
    return;
  }

  std::string GENIETune;
  if (cfg["tune"]) {
    GENIETune = cfg["tune"].as<std::string>();
  } else if (std::getenv("GENIE_XSEC_TUNE")) {
    GENIETune = std::getenv("GENIE_XSEC_TUNE");
  }

  if (!GENIETune.size()) {
    log_warn("No GENIE Tune set. Add \"tune\" key to configuration "
             "YAML node or set GENIE_XSEC_TUNE in the environment.");
    return;
  }

  log_debug("GHep3EventSource: SetTuneName({})", GENIETune);
  genie::RunOpt::Instance()->SetTuneName(GENIETune);

  if (cfg["event-generator-list"]) {
    EventGeneratorListName = cfg["event-generator-list"].as<std::string>();
  } else if (std::getenv("GENIE_XSEC_EVENTGENERATORLIST")) {
    EventGeneratorListName = std::getenv("GENIE_XSEC_EVENTGENERATORLIST");
  }

  log_debug("GHep3EventSource: SetEventGeneratorList({})",
            EventGeneratorListName);
  genie::RunOpt::Instance()->SetEventGeneratorList(EventGeneratorListName);

  genie::RunOpt::Instance()->BuildTune();

  genie::XmlParserStatus_t ist = splist->LoadFromXml(SplineXML);
  if (ist != genie::kXmlOK) {
    log_warn("genie::XsecSplineList failed to load from {}", SplineXML);
    return;
  }
}

std::shared_ptr<HepMC3::GenEvent> GHEP3EventSource::first() {

  if (!filepaths.size()) {
    return nullptr;
  }

  chin = std::make_unique<TChain>("gtree");

  for (auto const &ftr : filepaths) {
    if (!chin->Add(ftr.c_str(), 0)) {
      log_warn("Could not find gtree in {}", ftr.native());
      chin.reset();
      return nullptr;
    }
  }

  ch_ents = chin->GetEntries();
  ient = 0;

  if (ch_ents == 0) {
    return nullptr;
  }

  ntpl = NULL;
  auto branch_status = chin->SetBranchAddress("gmcrec", &ntpl);
  // should check this
  (void)branch_status;
  chin->GetEntry(0);

  ch_fuid = chin->GetFile()->GetUUID();
  ient = 0;
  auto ge = ghepconv::ToGenEvent(
      static_cast<genie::GHepRecord const &>(*ntpl->event));

  auto tpart = NuHepMC::Event::GetTargetParticle(*ge);
  auto bpart = NuHepMC::Event::GetBeamParticle(*ge);

  auto xspline = GetSpline(tpart->pid(), bpart->pid());

  gri = ghepconv::BuildRunInfo(xspline);

  ge->set_event_number(ient);
  ge->set_run_info(gri);
  ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);

  if (xspline) {
    auto xs = xspline->Evaluate(bpart->momentum().e() / ps::unit::GeV) /
              genie::units::pb;
    if (!std::isnormal(xs)) {
      log_debug("xs(E = {}, probe = {}, tgt = {}) = {}",
                bpart->momentum().e() / ps::unit::GeV, bpart->pid(),
                tpart->pid(), xs);
    } else {
      NUIS_LOG_TRACE("xs(E = {}, probe = {}, tgt = {}) = {}",
                     bpart->momentum().e() / ps::unit::GeV, bpart->pid(),
                     tpart->pid(), xs);
    }
    NuHepMC::EC2::SetTotalCrossSection(*ge, xs); // in GeV
  }

  return ge;
}

std::shared_ptr<HepMC3::GenEvent> GHEP3EventSource::next() {
  ient++;

  if (ient >= ch_ents) {
    return nullptr;
  }

  ntpl->Clear(); // this stops catastrophic memory leaks
  chin->GetEntry(ient);

  if (chin->GetFile()->GetUUID() != ch_fuid) {
    ch_fuid = chin->GetFile()->GetUUID();
  }

  auto ge = ghepconv::ToGenEvent(
      static_cast<genie::GHepRecord const &>(*ntpl->event));
  ge->set_event_number(ient);
  ge->set_run_info(gri);
  ge->set_units(HepMC3::Units::MEV, HepMC3::Units::CM);
  auto tpart = NuHepMC::Event::GetTargetParticle(*ge);
  auto bpart = NuHepMC::Event::GetBeamParticle(*ge);

  auto xspline = GetSpline(tpart->pid(), bpart->pid());
  if (xspline) {
    auto xs = xspline->Evaluate(bpart->momentum().e() / ps::unit::GeV) /
              genie::units::pb;
    if (!std::isnormal(xs)) {
      log_debug("xs(E = {}, probe = {}, tgt = {}) = {}",
                bpart->momentum().e() / ps::unit::GeV, bpart->pid(),
                tpart->pid(), xs);
    } else {
      NUIS_LOG_TRACE("xs(E = {}, probe = {}, tgt = {}) = {}",
                     bpart->momentum().e() / ps::unit::GeV, bpart->pid(),
                     tpart->pid(), xs);
    }
    NuHepMC::EC2::SetTotalCrossSection(*ge, xs); // in GeV
  }
  return ge;
}

genie::EventRecord const *
GHEP3EventSource::EventRecord(HepMC3::GenEvent const &ev) {
  auto ev_num = ev.event_number();
  if (ev_num != ient) { // don't read off disk if we don't need to
    ntpl->Clear();
    chin->GetEntry(ev_num);
  }
  return static_cast<genie::EventRecord const *>(ntpl->event);
}

IEventSourcePtr GHEP3EventSource::MakeEventSource(YAML::Node const &cfg) {
  return std::make_shared<GHEP3EventSource>(cfg);
}
GHEP3EventSource::~GHEP3EventSource() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::GHEP3EventSource::MakeEventSource, MakeEventSource);
#endif

} // namespace nuis
