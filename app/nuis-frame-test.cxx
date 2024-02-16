#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/frame/FrameGen.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"

using namespace nuis;

double enu(HepMC3::GenEvent const &ev) {
  return NuHepMC::Event::GetBeamParticle(ev)->momentum().e();
}

std::vector<double> enu_nupid(HepMC3::GenEvent const &ev) {
  auto beamp = NuHepMC::Event::GetBeamParticle(ev);
  return {beamp->momentum().e(), beamp->pid()};
}

struct single_procid_selector {
  int proc_id;
  single_procid_selector(int pid) : proc_id(pid) {}
  bool operator()(HepMC3::GenEvent const &ev){
    return (NuHepMC::ER3::ReadProcessID(ev) == proc_id);
  }
};


int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  auto frame = FrameGen(evs)
                   .Limit(50)
                   .AddColumn("procid",NuHepMC::ER3::ReadProcessID)
                   .AddColumns({"enu", "nupid"}, enu_nupid)
                   .Filter(single_procid_selector(500))
                   .Evaluate();
  std::cout << frame.Table << std::endl;
  std::cout << "NEvents Read:" <<  frame.norm_info.nevents << std::endl;
  std::cout << "NRows selected:" <<  frame.Table.rows() << std::endl;

  // auto fg =
  //     FrameGen(evs)
  //         .Limit(10000)
  //         .Filter([](auto const &ev) -> bool {
  //           return !(ev.event_number() % 1000);
  //         })
  //         .AddColumn("emu",
  //                    [](auto const &ev) -> double {
  //                      auto muon = NuHepMC::Event::GetParticle_First(
  //                          ev, NuHepMC::ParticleStatus::UndecayedPhysical,
  //                          {
  //                              13,
  //                          });
  //                      if (muon) {
  //                        return muon->momentum().e();
  //                      }
  //                      return 0xdeadbeef;
  //                    })
  //         .AddColumns(
  //             {"NFSNuc", "NFSPi"}, [](auto const &ev) -> std::vector<double>
  //             {
  //               auto fsnucleons =
  //                   NuHepMC::Event::GetParticles_AllRealFinalState(
  //                       ev, {2212, 2112});
  //               auto fspions =
  //               NuHepMC::Event::GetParticles_AllRealFinalState(
  //                   ev, {211, -211, 111});

  //               return {fsnucleons.size(), fspions.size()};
  //             });

  // auto evnums = fg.Evaluate();

  // size_t ci = 0;
  // for (auto &cn : evnums.ColumnNames) {
  //   std::cout << ci++ << ": " << cn << std::endl;
  // }
  // std::cout << evnums.Table << std::endl;

  // spdlog::info("Final FATX estimate: {}, after {} events, sumweights = {}",
  //              evnums.norm_info.fatx, evnums.norm_info.nevents,
  //              evnums.norm_info.sumweights);
}