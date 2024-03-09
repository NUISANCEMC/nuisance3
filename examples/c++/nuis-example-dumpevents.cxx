#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "nuis/log.txx"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace nuis;

using mylogger = nuis_named_log("mylogger");

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.make(argv[1]);

  if (!evs) {
    log_critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  NuHepMC::StatusCodeDescriptors procids;

  if (gri) {
    procids = NuHepMC::GR4::ReadProcessIdDefinitions(gri);
  }

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

    mylogger::log_info("Enu {}",
                       NuHepMC::Event::GetBeamParticle(ev)->momentum().e());

    mylogger::log_info("Event {}: Procid: {} = {}", ev.event_number(), procid,
                       procids[procid].first);
    mylogger::log_info("\tBeam particle: id = {}, E = {}", beamp->pid(),
                       beamp->momentum().e());
    mylogger::log_info("\tTarget particle id = {}", tgtp->pid());
    mylogger::log_info("\tnum FS protons = {}", nprot);
    mylogger::log_info("\tnum FS pi+ = {}", npip);
    mylogger::log_info("\tnum FS pi- = {}", npim);
    mylogger::log_info("\tnum FS pi0 = {}", npi0);
    mylogger::log_info("-------------------");

    if (ctr && !(ctr % 50000)) {
      mylogger::log_info("Processed {} events. FATX default estimate = {}", ctr,
                         evs->norm_info().fatx);
    }

    ctr++;
  }

  mylogger::log_info("Final FATX estimate: {}", evs->norm_info().fatx);
}