
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

#include "HepMC3/Print.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  YAML::Node hnode = YAML::Load(R"(
    type: HepMC3
    filepath: neut.hepmc3
    )");

  YAML::Node nnode = YAML::Load(R"(
    type: neutvect
    filepath: neut.neutvect.root
    )");

  auto hevs = fact.Make(hnode);
  auto nevs = fact.Make(nnode);

  auto isdiv3 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 3);
  };

  auto isdiv5 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 5);
  };

  for (auto ev : from(nevs).sel(isdiv3)) {
    std::cout << "neutvect event " << ev.event_number()
              << " passed isdiv3 filter." << std::endl;
    if (ev.event_number() > 20) {
      break;
    }
  }
  std::cout << "---" << std::endl;
  for (auto ev : from(nevs).sel(isdiv3).sel(isdiv5)) {
    std::cout << "neutvect event " << ev.event_number()
              << " passed isdiv3&&isdiv5 filter." << std::endl;
    if (ev.event_number() > 20) {
      break;
    }
  }
  std::cout << "---" << std::endl;
  for (auto [ks, ev] : from(nevs).multisel<std::string>(
           {{"isdiv3", isdiv3}, {"isdiv5", isdiv5}})) {
    std::cout << "neutvect event " << ev.event_number()
              << " passed isdiv3||isdiv5: " << std::endl;
    for (auto &f : ks) {
      std::cout << "\tpassed: " << f << std::endl;
    }
    if (ev.event_number() > 20) {
      break;
    }
  }
}