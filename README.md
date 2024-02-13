# eventinput

## Buildme

Some deps:
```bash
dnf install -y yaml-cpp-devel boost-devel fmt-devel spdlog-devel eigen3-devel
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

### python

For python bindings documentation see [here](src/python/README.md).

### C++

#### Read an event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto [gri, hevs] = fact.Make("neut.hepmc3");

  //Loop through events from an event source with a 
  // range-based for
  for (auto const &[ev,cvw] : hevs) {
    // do something with ev which is a HepMC3::GenEvent
    //   and cvw which is the NuhepMC G.R.7 CV weight for ev
  }
```

`gri` is a `std::shared_ptr<HepMC3::GenRunInfo>` for the corresponding event vector.

#### Estimate the FATX for an event source and use it to scale a histogram

```c++
#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/FATXUtils.hxx"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto [gri,nevs] = fact.Make("neut.hepmc3");

  //not a real histogram implementation, replace with your own
  hist myh;
  for (auto const &[ev,cvw] : nevs) {
    // do something with ev which is a HepMC3::GenEvent    
    myh.Fill(<some_ev_property>, cvw);
  }

  //Scale histogram to a flux-averaged differential cross-section prediction
  myh.Scale(nevs->norm_info().fatx()/nevs->norm_info().sumweights());
  myh.DivideBinsByWidth();
```

#### Read a NEUT neutvect event source

```c++
#include "nuis/eventinput/EventSourceFactory.h"

using namespace nuis;

int main() {

  EventSourceFactory fact;

  auto [gri,nevs] = fact.Make("neut.neutvect.root");

  for (auto const &[ev,cvw] : nevs) {
    // do something with ev which is a HepMC3::GenEvent converted on reading by the neutvectEventSource plugin
  }
```

This uses a dynamically-loaded plugin to convert an input neut file on the fly. You will need NEUT 5.8+ to be available at build configuration time for this feature to work.


