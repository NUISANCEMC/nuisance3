import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np
from contextlib import contextmanager,redirect_stderr,redirect_stdout
from os import devnull
from tqdm import tqdm 

neut_source = pn.EventSource("NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root")
#weightcalc = pn.WeightCalcFactory().make(neut_source, {"neut_cardname": "neut.card"})
#weightcalc.set_parameters(
#  {"MaCCQE": 0.0}
#)

def gen_comparison(cfg):
     rfact.make(anl_cfg).table(cfg["table"]).comparison()

rfact = pn.RecordFactory()
anl_cfg = {"type": "hepdata", "release": "ANL/CCQE/182176/", "table":"EventCounts-Q2"}
anl_comp = rfact.make(anl_cfg).table("EventCounts-Q2").comparison()




