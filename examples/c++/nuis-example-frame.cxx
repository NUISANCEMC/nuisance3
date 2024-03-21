#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/eventframe/EventFrameGen.h"

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
  bool operator()(HepMC3::GenEvent const &ev) {
    return (NuHepMC::ER3::ReadProcessID(ev) == proc_id);
  }
};

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;
  auto [gri, evs] = fact.make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  auto frame = EventFrameGen(evs)
                   .add_column("enu", enu)
                   .add_columns({"enu2", "pid"}, enu_nupid)
                   .progress(1E5)
                   .all();
}