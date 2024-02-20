import pytest

import numpy as np
import matplotlib.pyplot as plt
import os
import pyNUISANCE as nuis

# Make sure env variable is set
def test_check_env_set():
    assert os.getenv("NUISANCE_EVENT_PATH")
    assert os.getenv("NUISANCE_ROOT")

def test_create_nuwro_source():
    evs = nuis.EventSource("nuwro-sample-ANL.root")
    assert evs is not None
    assert evs.first() is not None
    assert evs.next() is not None

def test_nuwro_event_tuple():
   evs = nuis.EventSource("nuwro-sample-ANL.root")
   assert evs

   tupledata = evs.first()

   # Needs valid event, and CV = 1.0
   assert tupledata[0]
   assert tupledata[1] == 1.0

def test_nuwro_accumulator():
    evs = nuis.EventSource("nuwro-sample-ANL.root")
    flux_averaged_total_xs_estimate = evs.fatx()

    assert evs.fatx() > 0
    assert evs.sumw() > 0

    print(evs.fatx(), evs.sumw())

    for e, _ in evs:
        continue

    assert evs.fatx() > 0
    assert evs.sumw() > 0

    print(evs.fatx(), evs.sumw())

    assert evs.sumw() == 100.0


