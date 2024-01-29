
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>
using namespace boost::accumulators;

using namespace nuis;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto evs = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  auto gri = evs->run_info();

  NuHepMC::StatusCodeDescriptors procids;
  double FATX;

  // if (gri) {
  //   procids = NuHepMC::GR4::ReadProcessIdDefinitions(evs->run_info());

  //   spdlog::info("NuHepMC Version: {}",
  //                NuHepMC::GR2::ReadVersionString(evs->run_info()));

  //   FATX = NuHepMC::GC5::ReadFluxAveragedTotalXSec(evs->run_info());
  //   spdlog::info("FATX = {} ", FATX);
  //   if (NuHepMC::GC1::SignalsConvention(evs->run_info(), "G.C.7")) {
  //     for (auto &[beam_id, distrib] :
  //          NuHepMC::GC7::ReadAllEnergyDistributions(evs->run_info())) {
  //       spdlog::info("Have flux distribution for pid: {}.", beam_id);
  //     }
  //   }
  // }

  accumulator_set<double, stats<tag::sum_kahan>> totxs_recip;

  // auto some_evs = from(evs).atmost(1E6);

  size_t ctr = 0;
  for (auto const &ev : evs) {

    if(!ev.run_info()){
      // abort();
    }

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

    // totxs_recip(ev.weights().front() / NuHepMC::EC2::ReadTotalCrossSection(ev));

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
      spdlog::info("Processed {} events", ctr);
    }
    ctr++;
  }

  // auto totxs_recip_sum = sum_kahan(totxs_recip);
  // auto sumw = some_evs.sum_weights_so_far();

  // spdlog::info("sum(w/txs): {}, sum(w): {}, fatx estimate: {}", totxs_recip_sum, sumw,
  //              sumw / totxs_recip_sum);
}