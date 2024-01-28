
#include "nuis/eventinput/EventSourceFactory.h"

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

  auto isdiv7 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 7);
  };

  for (auto ev : nuis::Filter(hevs, isdiv7)) {
    std::cout << "hepmc3 ev passed filter: " << ev.event_number() << std::endl;
  }

  int i = 0;
  for (auto ev : nuis::Filter(nevs, isdiv7)) {
    std::cout << "neutvect passed filter: " << ev.event_number() << std::endl;
    if (i++ > 100) {
      break;
    }
  }
}