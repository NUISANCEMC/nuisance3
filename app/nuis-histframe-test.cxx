#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/frame/FrameGen.h"
#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/ROOTUtility.h"
#include "nuis/histframe/Utility.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "TCanvas.h"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

using namespace nuis;

double ToGeVFactor(HepMC3::GenEvent const &ev) {
  return (ev.momentum_unit() == HepMC3::Units::MEV) ? 1E-3 : 1;
}

double Q2_GeV(HepMC3::GenEvent const &ev) {
  auto beamp = NuHepMC::Event::GetBeamParticle(ev);
  auto beam_pid = beamp->pid();
  auto cc_lep_pid = (beam_pid > 0) ? (beam_pid - 1) : (beam_pid + 1);

  auto lep = NuHepMC::Event::GetParticle_First(
      ev, NuHepMC::ParticleStatus::UndecayedPhysical, {cc_lep_pid});

  return -(beamp->momentum() - lep->momentum()).m2() *
         std::pow(ToGeVFactor(ev), 2);
}

double q0_GeV(HepMC3::GenEvent const &ev) {
  auto beamp = NuHepMC::Event::GetBeamParticle(ev);
  auto beam_pid = beamp->pid();
  auto cc_lep_pid = (beam_pid > 0) ? (beam_pid - 1) : (beam_pid + 1);

  auto lep = NuHepMC::Event::GetParticle_First(
      ev, NuHepMC::ParticleStatus::UndecayedPhysical, {cc_lep_pid});

  return (beamp->momentum() - lep->momentum()).e() * ToGeVFactor(ev);
}

int isCC(HepMC3::GenEvent const &ev) {
  auto beam_pid = NuHepMC::Event::GetBeamParticle(ev)->pid();
  auto cc_lep_pid = (beam_pid > 0) ? (beam_pid - 1) : (beam_pid + 1);

  auto all_cc_lep =
      NuHepMC::Event::GetParticles_AllRealFinalState(ev,
                                                     {
                                                         cc_lep_pid,
                                                     })
          .size();
  auto all_cc_lepbar =
      NuHepMC::Event::GetParticles_AllRealFinalState(ev,
                                                     {
                                                         -cc_lep_pid,
                                                     })
          .size();

  return (all_cc_lep - all_cc_lepbar) == 1;
}

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;
  auto [gri, evs] = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }
  nuis::HistFrame q0(Bins::LinSpace(100, 0, 10, "q_0 [GeV]"));
  auto ccqe_col = q0.AddColumn("CCQE");
  auto MEC_col = q0.AddColumn("MEC");
  auto RES_col = q0.AddColumn("RES");

  size_t i = 0;
  for (auto const &[ev, cvw] : evs) {
    if (!isCC(ev)) {
      continue;
    }
    q0.Fill(q0_GeV(ev), cvw);
    auto proc_id = NuHepMC::ER3::ReadProcessID(ev);
    if ((proc_id >= 200) && (proc_id < 250)) {
      q0.Fill(q0_GeV(ev), cvw, ccqe_col);
    } else if ((proc_id >= 300) && (proc_id < 350)) {
      q0.Fill(q0_GeV(ev), cvw, MEC_col);
    } else if ((proc_id >= 400) && (proc_id < 450)) {
      q0.Fill(q0_GeV(ev), cvw, RES_col);
    }
  }

  auto norm_info = evs->norm_info();

  ScaleAllMC(q0, norm_info.fatx / norm_info.sumweights, true);

  auto h1 = ToTH1(q0, "q0hist");
  auto h1_CCQE = ToTH1(q0, "q0hist_CCQE", ccqe_col);
  auto h1_MEC = ToTH1(q0, "q0hist_MEC", MEC_col);
  auto h1_RES = ToTH1(q0, "q0hist_RES", RES_col);

  TCanvas c1("c1", "");
  h1->Draw();
  h1_CCQE->Draw("HIST SAME");
  h1_MEC->Draw("HIST SAME");
  h1_RES->Draw("HIST SAME");
  c1.Print("mycanv.pdf");
}