import pytest

import numpy as np
import matplotlib.pyplot as plt

import pyEVENTINPUT as ev
import pyMEASUREMENT as me
import pyNuisAnalysis as nuis
from pyHepMC3 import HepMC3 as hm

# Some of these tests right now just check a 
# piece of code runs without aborting..

# Initialize test environment
nuis.configure()

@pytest.fixture
def measurement_setup_1():
    return { 
        "measurement": "ANL_Analysis_mycustomtag",
        "release": "ANL/CCQE/182176/",
        "table": "EventCounts-Q2"
    }

@pytest.fixture
def hepdata_handler(measurement_setup_1):
    return me.Measurement(measurement_setup_1)

def hepdata_createrecord(hepdata_handler):
    record = hepdata_handler.CreateRecord("")
    assert record is not None

def hepdata_finalizerecord(hepdata_handler):
    record = hepdata_handler.CreateRecord("")
    hepdata_handler.FinalizeRecord(record)

@pytest.fixture
def nuwro_source_fixture():
    return ev.EventSource("../../data/test_event_samples/nuwro-sample-ANL.root")

@pytest.fixture
def nuwro_event_fixture(nuwro_source_fixture):
    return nuwro_source_fixture.first()

def test_create_records(hepdata_handler):
    data = hepdata_handler.CreateRecord("")
    assert data is not None
    assert data.mc_counts is not None
    assert data.mc_weights is not None
    assert data.data_value is not None

    data = hepdata_handler.CreateRecord("data")
    assert data is not None


def test_measurement_nuwro_proj(hepdata_handler, nuwro_event_fixture):
    proj = hepdata_handler.ProjectEvent(nuwro_event_fixture)
    assert proj is not None

def test_measurement_nuwro_filter(hepdata_handler, nuwro_event_fixture):
    signal = hepdata_handler.FilterEvent(nuwro_event_fixture)
    assert signal is not None

def test_fill_records_from_nuwro_source(hepdata_handler, nuwro_source_fixture):
    nuwro_source = nuwro_source_fixture
    nuwro_record = hepdata_handler.CreateRecord("comp_nuwro")

    # Fill records from events
    for e in nuwro_source:
        if not hepdata_handler.FilterEvent(e): continue
        proj = hepdata_handler.ProjectEvent(e)
        nuwro_record.FillBinFromProjection(proj, 1.0)

    # Assert records are filled
    assert nuwro_record.mc_counts is not None
    # assert np.sum(nuwro_record.mc_counts) != 0




# def test_plot_results(hepdata_handler):
#     nuwro_record = hepdata_handler.CreateRecord("anl_comp_nuwro")
#     nuwro_record = hepdata_handler.CreateRecord("anl_comp_neut")

#     # Plot results
#     plt.xkcd()
#     plt.errorbar(x=nuwro_record.x(), y=nuwro_record.mc(), xerr=0.0, yerr=0.0, label="NUWRO")
#     plt.errorbar(x=nuwro_record.x(), y=nuwro_record.mc(), xerr=0.0, yerr=0.0, label="NEUT")
#     plt.xlabel("Q2")
#     plt.ylabel("Events")
#     plt.legend()
#     plt.savefig("test.png")

#     # Assert the plot is saved
#     assert "test.png" in os.listdir()




# NUISANCE python interface

## EventInput

The `pyEventInput` modules wraps the NUISANCE event reading functionality.

We generally don't recommend writing event loops directly in python because python looping is very slow, however, it can be useful for prototyping plots or selections.

A simple, unweighted event loop might look like below:

```python
import pyEventInput as ei

evs = ei.EventSource("path/to/my/inputfile")

if not evs.good():
  print("Failed to open input file")
  exit(1)

for ev, _ in evs:
  # Do something with each event, ev, which are HepMC3::GenEvents
```

`pyEventInput.EventSource` is an interface to [`nuis::EventSourceFactory`](../eventinput/EventSourceFactory.h), which is able to use a number of plugins to try and read an input file and present it as a HepMC3 event vector adhereing to the [NuHepMC](https://github.com/NuHepMC/Spec) specification.

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
