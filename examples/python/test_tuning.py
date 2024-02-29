import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np
from contextlib import contextmanager,redirect_stderr,redirect_stdout
from os import devnull
from tqdm import tqdm 
pn.configure()

from io import StringIO  # Python 3
import sys

def stop_talking():
  temp_out = StringIO()
  sys.stdout = temp_out

def start_talking():
  sys.stdout = sys.__stdout__

neut_source = pn.EventSource("NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root")
weightcalc = pn.WeightCalcFactory().Make(neut_source, {"neut_cardname": "neut.card"})

weightcalc.SetParameters(
  {"MaCCQE": 0.0}
)

anl_handler = pn.Measurement(
  { "measurement": "ANL", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}
)

def LikelihoodEvaluation(x):

  parset = {"MaCCQE": x}
  weightcalc.SetParameters(
    parset
  )

  original = anl_handler.CreateProjection("test")

  prefilt = pn.ps.filter.ANL_CCQE_182176_Filter
  preproj = pn.ps.project.ANL_CCQE_182176_Project_Q2
  count = 0
  for (e, cvw) in tqdm (neut_source, desc=f"[INFO] : Processing ParSet {parset}"):
    # Original fill
    f = prefilt(e)
    p = preproj(e)
    stop_talking()
    original.Fill(f, [p], weightcalc(e))
    start_talking()

    count += 1
    if count > 10000: break

  scaling  = np.sum(original["data"].value)/np.sum(original["mc"].value)
  residual = original["data"].value - scaling * original["mc"].value

  # Simple for now
  chi2 = np.sum(residual*residual)

  return chi2

chi2data = []
for v in np.linspace(-2.0,2.0,11):

  chi2data.append(LikelihoodEvaluation(v))

print(chi2data)