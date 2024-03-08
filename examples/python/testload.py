import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np
from tqdm import tqdm
import numba
pn.configure()

nuwro_source = pn.EventSource("../../../runs/NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")

@numba.jit(nopython=False)
def q3(evt):
    print("CALLING")
    mu = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kMuon)
    nu = pn.ps.sel.Beam(evt, pn.ps.pdg.kNuMu)
    part = pn.ps.proj.parts.q3(nu,mu)
    if not mu or not nu: return 0
    return part

for evt,_ in nuwro_source:
    q3(evt)
    break
    
# print("FRAME Evaluation")
fr = pn.FrameGen(nuwro_source, 100000).Limit(1000)
fr.AddColumn("q3", q3)

df = fr.Evaluate()

print(df)
