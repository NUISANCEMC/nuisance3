#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"
#include "nuis/weightcalc/WeightCalcFunc.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace nuis;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  NuHepMC::StatusCodeDescriptors procids;

  if (gri) {
    procids = NuHepMC::GR4::ReadProcessIdDefinitions(gri);
  }

  nuis::WeightCalcFactory wfact;

  YAML::Node neut_reweight_node = YAML::Load(R"(
    neut_cardname: "neut.card"
    )");

  auto weighters = wfact.Make(evs, neut_reweight_node);
  weighters->SetParameters({
      {"MaCCQE", -2},
  });

  size_t ctr = 0;
  for (auto const &[ev, w] : evs) {

    auto beamp = NuHepMC::Event::GetBeamParticle(ev);
    auto tgtp = NuHepMC::Event::GetTargetParticle(ev);
    auto nprot = NuHepMC::Event::GetParticles_All(
                     ev, NuHepMC::ParticleStatus::UndecayedPhysical,
                     {
                         2212,
                     })
                     .size();
    auto npip = NuHepMC::Event::GetParticles_All(
                    ev, NuHepMC::ParticleStatus::UndecayedPhysical,
                    {
                        211,
                    })
                    .size();
    auto npim = NuHepMC::Event::GetParticles_All(
                    ev, NuHepMC::ParticleStatus::UndecayedPhysical,
                    {
                        -211,
                    })
                    .size();
    auto npi0 = NuHepMC::Event::GetParticles_All(
                    ev, NuHepMC::ParticleStatus::UndecayedPhysical,
                    {
                        111,
                    })
                    .size();

    auto procid = NuHepMC::ER3::ReadProcessID(ev);

    // spdlog::info("Enu {}, wgtm {}, wgtv {}",
    //              NuHepMC::Event::GetBeamParticle(ev)->momentum().e(),
    //              weighter_m.CalcWeight(ev), weighter_v.CalcWeight(ev));

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

    // spdlog::info("evt: mode {}, wght GENIEReWeight: {}",
    // procids[procid].first,
    //              weighters->CalcWeight(ev));

    if (ctr && !(ctr % 50000)) {
      spdlog::info("Processed {} events. FATX default estimate = {}", ctr,
                   evs->norm_info().fatx);
    }

    ctr++;
  }

  spdlog::info("Final FATX estimate: {}", evs->norm_info().fatx);
}