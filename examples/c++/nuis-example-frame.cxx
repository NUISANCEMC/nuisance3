#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/eventframe/EventFrameGen.h"

#include "nuis/histframe/newfill.h"
#include "nuis/histframe/utility.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"

using namespace nuis;

double enu(HepMC3::GenEvent const &ev) {
  return NuHepMC::Event::GetBeamParticle(ev)->momentum().e() / 1E3;
}

int categ_enub(HepMC3::GenEvent const &ev) {
  auto enu = NuHepMC::Event::GetBeamParticle(ev)->momentum().e() / 1E3;
  if (enu < 2) {
    return 0;
  }
  if (enu < 5) {
    return 1;
  }
  if (enu < 7) {
    return 2;
  }
  return 3;
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

  auto fg = EventFrameGen(evs)
                .add_column("enu", enu)
                .add_column("categ_enub", categ_enub)
                .add_columns({"enu2", "pid"}, enu_nupid)
                .progress(1E5);

  auto enuh1 = HistFrame(Binning::lin_space(0, 10, 10));
  auto enuh2 = HistFrame(Binning::lin_space(0, 10, 10));
  auto enuh3 = HistFrame(Binning::lin_space(0, 10, 10));
  auto enuh4 = HistFrame(Binning::lin_space(0, 10, 10));
  enuh4.add_column("a");
  enuh4.add_column("b");
  enuh4.add_column("c");
  auto enuh5 = HistFrame(Binning::lin_space(0, 10, 10));
  auto enuh6 = HistFrame(Binning::lin_space(0, 10, 10));
  auto enuh7 = HistFrame(Binning::lin_space(0, 10, 10));

  auto chunk = fg.first();
  while (chunk) {
    fill(enuh1, chunk, {"enu"});
    fill(enuh2, chunk, {"enu"}, fill_column(0));
    fill(enuh3, chunk, {"enu"}, fill_column("newcol"));
    fill(enuh4, chunk, {"enu"}, categorize_by("categ_enub"));
    fill(enuh5, chunk, {"enu"}, categorize_by("categ_enub", {"enu < 2", "2 < enu < 5", "5 < enu < 7"}));
    fill(enuh6, chunk, {"enu"}, weighted_column_map("categ_enub"));
    fill(enuh7, chunk, {"enu"}, split_by_ProcID());
    chunk = fg.next();
  }
  std::cout << "enu1\n"<< enuh1 << "\n\n\n" << std::endl;
  std::cout << "enu2\n"<< enuh2 << "\n\n\n" << std::endl;
  std::cout << "enu3\n"<< enuh3 << "\n\n\n" << std::endl;
  std::cout << "enu4\n"<< enuh4 << "\n\n\n" << std::endl;
  std::cout << "enu5\n"<< enuh5 << "\n\n\n" << std::endl;
  std::cout << "enu6\n"<< enuh6 << "\n\n\n" << std::endl;
  std::cout << "enu7\n"<< enuh7 << "\n\n\n" << std::endl;
}