import numpy as np
import matplotlib.pyplot as plt
import sys
import pyNUISANCE as pn

pn.configure()

nuwro_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not nuwro_source: sys.exit()

def eventproc(ev):
  return 0.2

fr = pn.FrameGen(nuwro_source, 50000).Limit(200)
fr.AddColumn("eventproc", eventproc)
fr.Limit(49000)
df = fr.Evaluate()


print(df.Table)
