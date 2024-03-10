# NUISANCE python interface

## Jupyter Notebook Tutorial

You can follow the notebook tutorial that covers the main features of the python API over the following notebooks:

1. [Opening Event Files](notebooks/opening_event_files.ipynb)
1. [HepMC3 Binding Interoperability](notebooks/HepMC3_binding_interop.ipynb)
1. [Basic Event Loops](notebooks/basing_loops.ipynb)
1. [Advanced Event Loops](notebooks/advanced_looping.ipynb)
1. [HistFrames](notebooks/histframes.ipynb)

There are also a few short notebooks for miscellania:

1. [Getting NuHepMC Flux Information](notebooks/flux_info.ipynb)
1. [Simple Oscillation Weights](notebooks/osc.ipynb)
1. [Altering Logging Levels](notebooks/logging.ipynb)

## API Walking Tour

### EventInput

The `pyNUISANCE.EventSource` interface wraps the NUISANCE event reading functionality.

We generally don't recommend writing event loops directly in python because python looping is very slow, however, it can be useful for prototyping plots or selections. For a faster loop, see [Frame](#frame).

A simple, unweighted event loop might look like below:

```python
from pyNUISANCE import EventSource

evs = EventSource("path/to/my/inputfile")

if not evs:
  print("Failed to open input file")
  exit(1)

for ev, _ in evs:
  # Do something with each event, ev, which are HepMC3::GenEvents
```

`pyNUISANCE.EventSource` is an interface to [`nuis::EventSourceFactory`](../eventinput/EventSourceFactory.h), which is able to use a number of plugins to try and read an input file and present it as a HepMC3 event vector adhereing to the [NuHepMC](https://github.com/NuHepMC/Spec) specification.

Iterating on an event source returns a tuple of an event and its Central Value (CV) weight (as defined in [NuHepMC G.R.7](https://github.com/NuHepMC/Spec?tab=readme-ov-file#gr7-event-weights)). In the above example, the CV weight is ignored in the loop variables. All correctly normalised cross-section predictions must weight each event's properties by the CV weight. After looping through all of the events, the best-estimate of the flux-averaged total cross section (FATX) and the sum of CV weights can be queries. N.B. the internal accumulation of these properties are reset for each loop for consistency, so they should only be used after looping.

```python
for ev, cvw in evs:
  # Do something with each event, ev, and the associated CV weight, cvw

flux_averaged_total_xs_estimate = evs.fatx()
sum_cvweights = evs.sumw()
```

You do not need to loop over all events in a file and breaking early will leave the FATX accumulator in a good state, which lets you prototype accurately with a smaller number of events, and scale up to more events without any special handling of any cross-section normalizations. You can even print out the current FATX estimate and the sumw as you go:

```python
for i, (ev, cvw) in enumerate(evs):
  if i and (((i+1)% 50000) == 0):
    print("On event %s, CV weight = %s, FATX best estimate = %s pb, sum CV weights = %s " \
      % (i, cvw, evs.fatx(), evs.sumw()))
```

The above might give output like:

```
On event 49999, CV weight = 1.0, FATX best estimate = 0.884602612 pb, sum CV weights = 50000.0 
On event 99999, CV weight = 1.0, FATX best estimate = 0.877038888 pb, sum CV weights = 100000.0 
On event 149999, CV weight = 1.0, FATX best estimate = 0.875219239 pb, sum CV weights = 150000.0 
On event 199999, CV weight = 1.0, FATX best estimate = 0.875539752 pb, sum CV weights = 200000.0 
On event 249999, CV weight = 1.0, FATX best estimate = 0.875984252 pb, sum CV weights = 250000.0 
On event 299999, CV weight = 1.0, FATX best estimate = 0.876886361 pb, sum CV weights = 300000.0 
On event 349999, CV weight = 1.0, FATX best estimate = 0.876789265 pb, sum CV weights = 350000.0
```
