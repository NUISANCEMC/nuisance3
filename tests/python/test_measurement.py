import pytest

import numpy as np
import matplotlib.pyplot as plt

import pyNUISANCE as nuis

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
    return nuis.Measurement(measurement_setup_1)

def hepdata_createrecord(hepdata_handler):
    record = hepdata_handler.CreateRecord("")
    assert record is not None

def hepdata_finalizerecord(hepdata_handler):
    record = hepdata_handler.CreateRecord("")
    hepdata_handler.FinalizeRecord(record)

@pytest.fixture
def nuwro_source_fixture():
    return nuis.EventSource("../../data/test_event_samples/nuwro-sample-ANL.root")

@pytest.fixture
def nuwro_event_fixture(nuwro_source_fixture):
    ev, _ = nuwro_source_fixture.first()
    return ev

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
    print(nuwro_event_fixture)
    signal = hepdata_handler.FilterEvent(nuwro_event_fixture)
    assert signal is not None

def test_fill_records_from_nuwro_source(hepdata_handler, nuwro_source_fixture):
    nuwro_source = nuwro_source_fixture
    nuwro_record = hepdata_handler.CreateRecord("comp_nuwro")

    if not nuwro_source_fixture.good():
        print("Failed to open input file")
        exit(1)

    # Fill records from events
    for data in nuwro_source:
        print(data)
        if not hepdata_handler.FilterEvent(data[0]): continue
        proj = hepdata_handler.ProjectEvent(data[0])
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

