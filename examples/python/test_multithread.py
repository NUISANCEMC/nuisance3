import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np
from tqdm import tqdm
import threading
import concurrent.futures

# PRO SELECTA TESTS
pn.configure()
pn.ps.LoadFile("../../data/neutrino_data/ANL/CCQE/182176/analysis.cxx")

print(pn.ps.project.ANL_CCQE_182176_Project_Q2)

anlsetup = { "measurement": "ANL_Analysis_mycustomtag",
                "release": "ANL/CCQE/182176/",
                "table": "EventCounts-Q2"}
anl_handler = pn.Measurement(anlsetup)

def thread_check(filename, handler):

    original = handler.CreateProjection("original")
    neut_source = pn.EventSource(filename)

    for (e, cvw) in tqdm (neut_source, desc="[INFO] : Processing Record "):
        f = handler.FillProjectionFromEvent( original, e )
    
    return original["mc"].value

master_list = [] # This list will hold all the generated lists

# Using ThreadPoolExecutor to execute `generate_list` function in parallel
with concurrent.futures.ThreadPoolExecutor() as executor:

    futures = []
    for filename in [
        "NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root",
        "NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root"
    ]:
        futures.append( executor.submit(thread_check, filename, anl_handler ))
    
    # Collecting results as they are completed
    for future in concurrent.futures.as_completed(futures):
        # Adding the result (a list) to the master list
        master_list.append(future.result())

print(master_list)


# Using ThreadPoolExecutor to execute `generate_list` function in parallel
with concurrent.futures.ThreadPoolExecutor() as executor:

    futures = []
    for filename in [
        "NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root",
        "NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root"
    ]:
        futures.append( executor.submit(thread_check, filename, anl_handler ))
    
    # Collecting results as they are completed
    for future in concurrent.futures.as_completed(futures):
        # Adding the result (a list) to the master list
        master_list.append(future.result())

print(master_list)