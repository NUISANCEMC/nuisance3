#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/frame/Frame.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"

using namespace nuis;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  auto fg =
      FrameGen(evs)
          .Limit(20)
          .Filter(
              [](auto const &ev) -> bool { return !(ev.event_number() % 3); })
          .AddColumn("emu",
                     [](auto const &ev) -> double {
                       auto muon = NuHepMC::Event::GetParticle_First(
                           ev, NuHepMC::ParticleStatus::UndecayedPhysical,
                           {
                               13,
                           });
                       if (muon) {
                         return muon->momentum().e();
                       }
                       return 0xdeadbeef;
                     })
          .AddColumns(
              {"NFSNuc", "NFSPi"}, [](auto const &ev) -> std::vector<double> {
                auto fsnucleons =
                    NuHepMC::Event::GetParticles_AllRealFinalState(
                        ev, {2212, 2112});
                auto fspions = NuHepMC::Event::GetParticles_AllRealFinalState(
                    ev, {211, -211, 111});

                return {fsnucleons.size(), fspions.size()};
              });

  auto evnums = fg.Evaluate();
  size_t ci = 0;
  for (auto &cn : evnums.ColumnNames) {
    std::cout << ci++ << ": " << cn << std::endl;
  }
  std::cout << evnums.Table << std::endl;

  spdlog::info("Final FATX estimate: {}", evs->norm_info().fatx());
}