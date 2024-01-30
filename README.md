# eventinput

## Buildme

Some deps:
```bash
dnf install -y yaml-cpp-devel boost-devel fmt-devel spdlog-devel
```

build like:

```bash
cd /path/to/repo
mkdir build; cd build
cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo
make install -j $(nproc)
```

source the env

```bash
source Linux/setup.nuis-eventinput.sh
printenv | grep -i nuisance
```

#### Read an event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto hevs = fact.Make("neut.hepmc3");

  //Loop through events from an event source with a 
  // range-based for
  for (auto const &ev : hevs) {
    // do something with ev which is a HepMC3::GenEvent
  }
```

#### Fetch the HepMC3::GenRunInfo from an event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto hevs = fact.Make("neut.hepmc3");

  //The for loop always starts from the first event
  // so no need to worry about having called next to 
  // get a pointer to the run_info
  auto gri = hevs->next().run_info();

  for (auto const &ev : hevs) {
    // do something with ev which is a HepMC3::GenEvent
  }
```

#### Estimate the FATX for an event source and use it to scale a histogram

```c++
#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/FATXUtils.hxx"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto nevs = fact.Make("neut.hepmc3");

  auto FATXAcc = NuHepMC::FATX::MakeAccumulator(nevs->next().run_info());

  //not a real histogram implementation, replace with your own
  hist myh;
  for (auto const &ev : nevs) {
    // do something with ev which is a HepMC3::GenEvent
    FATXAcc->process(ev);
    
    myh.Fill(<some_property>, ev.weights("CV"));
  }

  double FATX_best_estimate = FATXAcc->fatx();

  //Scale histogram to a flux-averaged differential cross-section prediction
  myh.Scale(FATX_best_estimate/FATXAcc->sumweights());
  myh.DivideBinsByWidth();
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

##### N.B. An rvalue Gotcha!

It may look like you can use the temporary return value from the tail-called operators like below:

```c++
for(auto const & ev : from(fact.Make("neut.hepmc3")).sel(isdiv5)){
  //do something with ev
}
```

but there are gotcha's abound therein, so we recommend that you keep a handle to the fully 
tail-called object and then loop on that handle, like so:

```c++
auto evlooper = from(fact.Make("neut.hepmc3")).sel(isdiv5);
for(auto const & ev :evlooper ){
  //do something with ev
}
```

