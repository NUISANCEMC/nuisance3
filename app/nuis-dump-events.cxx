#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

#include "nuis/weightcalc/WeightCalcFunc.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

using namespace nuis;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto evs = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  auto gri = evs->first().value().run_info();

  NuHepMC::StatusCodeDescriptors procids;
  double FATX;

  // if (gri) {
  //   procids = NuHepMC::GR4::ReadProcessIdDefinitions(gri);

  //   spdlog::info("NuHepMC Version: {}",
  //   NuHepMC::GR2::ReadVersionString(gri));

  //   FATX = NuHepMC::GC5::ReadFluxAveragedTotalXSec(gri);
  //   spdlog::info("FATX = {} ", FATX);
  //   if (NuHepMC::GC1::SignalsConvention(gri, "G.C.7")) {
  //     for (auto &[beam_id, distrib] :
  //          NuHepMC::GC7::ReadAllEnergyDistributions(gri)) {
  //       spdlog::info("Have flux distribution for pid: {}.", beam_id);
  //     }
  //   }
  // }

  auto FATXAcc = NuHepMC::FATX::MakeAccumulator(gri);

  auto weighter_v = nuis::WeightCalcFuncHM3(
      [](HepMC3::GenEvent const &ev, std::vector<double> const &p) {
        return NuHepMC::Event::GetBeamParticle(ev)->momentum().e() * p.at(1);
      });
  weighter_v.SetParameters({1, 0.01});

  auto weighter_m = nuis::WeightCalcFuncHM3_NamedParams(
      [](HepMC3::GenEvent const &ev, std::map<std::string, double> const &p) {
        return NuHepMC::Event::GetBeamParticle(ev)->momentum().e() *
               p.at("eweight");
      });

  weighter_m.SetParameters({{"eweight", 0.1}});

  size_t ctr = 0;
  for (auto const &ev : evs) {

    // auto beamp = NuHepMC::Event::GetBeamParticle(ev);
    // auto tgtp = NuHepMC::Event::GetTargetParticle(ev);
    // auto nprot = NuHepMC::Event::GetParticles_All(
    //                  ev, NuHepMC::ParticleStatus::UndecayedPhysical,
    //                  {
    //                      2212,
    //                  })
    //                  .size();
    // auto npip = NuHepMC::Event::GetParticles_All(
    //                 ev, NuHepMC::ParticleStatus::UndecayedPhysical,
    //                 {
    //                     211,
    //                 })
    //                 .size();
    // auto npim = NuHepMC::Event::GetParticles_All(
    //                 ev, NuHepMC::ParticleStatus::UndecayedPhysical,
    //                 {
    //                     -211,
    //                 })
    //                 .size();
    // auto npi0 = NuHepMC::Event::GetParticles_All(
    //                 ev, NuHepMC::ParticleStatus::UndecayedPhysical,
    //                 {
    //                     111,
    //                 })
    //                 .size();

    // auto procid = NuHepMC::ER3::ReadProcessID(ev);

    FATXAcc->process(ev);
    spdlog::info("Enu {}, wgtm {}, wgtv {}",
                 NuHepMC::Event::GetBeamParticle(ev)->momentum().e(),
                 weighter_m(ev), weighter_v(ev));

    // spdlog::info("Event {}: Procid: {} = {}", ev.event_number(), procid,
    //              procids[procid].first);
    // spdlog::info("\tBeam particle: id = {}, E = {}", beamp->pid(),
    //              beamp->momentum().e());
    // spdlog::info("\tTarget particle id = {}", tgtp->pid());
    // spdlog::info("\tnum FS protons = {}", nprot);
    // spdlog::info("\tnum FS pi+ = {}", npip);
    // spdlog::info("\tnum FS pi- = {}", npim);
    // spdlog::info("\tnum FS pi0 = {}", npi0);
    // spdlog::info("-------------------");

    if (ctr && !(ctr % 50000)) {
      spdlog::info("Processed {} events. FATX default estimate = {}", ctr,
                   FATXAcc->fatx());
    }
    ctr++;
  }

  spdlog::info("Final FATX estimate: {}", FATXAcc->fatx());
}