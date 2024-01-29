# eventinput

Read an event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  YAML::Node hnode = YAML::Load(R"(
    type: HepMC3
    filepath: neut.hepmc3
    )");
  
  auto hevs = fact.Make(hnode);

  for (auto ev : nevs) {
    // do something with ev which is a HepMC3::GenEvent
  }
```

Select every 5th event:

```c++
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  YAML::Node hnode = YAML::Load(R"(
    type: HepMC3
    filepath: neut.hepmc3
    )");
  
  auto hevs = fact.Make(hnode);
  auto isdiv5 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 5);
  };

  for (auto ev : from(nevs).sel(isdiv5)) {
    // do something with every 5th ev which is a HepMC3::GenEvent
  }
```

Select every 3rd and 5th event and track which selections each event passes:

```c++
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  YAML::Node hnode = YAML::Load(R"(
    type: HepMC3
    filepath: neut.hepmc3
    )");
  
  auto hevs = fact.Make(hnode);
  auto isdiv3 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 3);
  };
  auto isdiv5 = [](HepMC3::GenEvent const &ev) -> bool {
    return !(ev.event_number() % 5);
  };

  //templated on selection function key type
  for (auto [ks,ev] : from(nevs).multisel<std::string>({{"3",isdiv3},{"5",isdiv5}})) {
    // do something with every 3rd and 5th ev which is a HepMC3::GenEvent
    // is a std::vector<std::string> of the keys of selection functions that ev passes.
  }
```