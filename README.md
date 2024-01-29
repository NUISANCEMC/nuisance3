# eventinput

#### Read an event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto hevs = fact.Make("neut.hepmc3");

  for (auto const &ev : hevs) {
    // do something with ev which is a HepMC3::GenEvent
  }
```

#### Read a NEUT neutvect event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto nevs = fact.Make("neut.neutvect.root");

  for (auto const &ev : nevs) {
    // do something with ev which is a HepMC3::GenEvent
  }
```

This uses a dynamically-loaded plugin to convert an input neut file on the fly.

#### Select every 5th event

```c++
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto isdiv5 = [](auto const &ev) -> bool {
    return !(ev.event_number() % 5);
  };

  auto hevs = from(fact.Make("neut.hepmc3")).sel(isdiv5);

  for (auto const &ev : hevs) {
    // do something with every 5th ev which is a HepMC3::GenEvent
  }
```

#### Select every 3rd and 5th event and track which selections each event passes:

```c++
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto isdiv3 = [](auto const &ev) -> bool {
    return !(ev.event_number() % 3);
  };
  auto isdiv5 = [](auto const &ev) -> bool {
    return !(ev.event_number() % 5);
  };


  //multisel call is templated on selection function key type
  auto hevs = from(fact.Make("neut.hepmc3")).multisel<std::string>(
    {{"3",isdiv3},{"5",isdiv5}});

  for (auto &[ks,ev] : hevs) {
    // do something with every 3rd and 5th ev which is a HepMC3::GenEvent
    // is a std::vector<std::string> of the keys of selection functions that ev passes.
  }
```

#### Only run on the first 100 events

```c++
#include "nuis/eventinput/EventSourceFactory.h"
#include "nuis/eventinput/FilteredEventSource.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;
  
  auto isdiv3 = [](auto const &ev) -> bool {
    return !(ev.event_number() % 3);
  };
  auto isdiv5 = [](auto const &ev) -> bool {
    return !(ev.event_number() % 5);
  };

  auto hevs = from(fact.Make("neut.hepmc3")).atmost(100).some_evs.multisel<std::string>({{"3",isdiv3},{"5",isdiv5}});

  //templated on selection function key type
  for (auto &[ks,ev] : hevs) {
    // do something with every 3rd and 5th ev which is a HepMC3::GenEvent
    // is a std::vector<std::string> of the keys of selection functions that ev passes.
  }

  auto nevents_read = hevs.events_read();
```

But usually you really want to know the sum of weights rather than the number of events:

```c++
  auto sumw = hevs.sum_weights_so_far();
```

##### N.B. A rvalue Gotcha!

It may look like you can use the temporary return value from the tail-called operators like below:

```c++
for(auto const & ev : from(fact.Make("neut.hepmc3")).sel(isdiv5)){
  //do something with ev
}
```

but there are gotcha's abound therein, so we recommend that you keep a handle to the fully 
tail-called object and then loop on that handle.

