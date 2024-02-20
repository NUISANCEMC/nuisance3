import numpy as np
import matplotlib.pyplot as plt
import sys
import pyNUISANCE as pn

pn.configure()

nuwro_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not nuwro_source: sys.exit()

neut_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not neut_source: sys.exit()

anlsetup = { "measurement": "ANL_Analysis_mycustomtag",
            "release": "ANL/CCQE/182176/",
            "table": "EventCounts-Q2"}

anl_handler = pn.Measurement(anlsetup)
anl_nuwro = anl_handler.CreateProjection("anl_comp_nuwro")
anl_neut  = anl_handler.CreateProjection("anl_comp_neut")

for e, cvw in neut_source:
    if not anl_handler.FilterEvent(e): continue
    proj = anl_handler.ProjectEvent(e)
    anl_neut.FillBinFromProjection( proj, cvw )

for e, cvw in nuwro_source:
    if not anl_handler.FilterEvent(e): continue
    proj = anl_handler.ProjectEvent(e)
    anl_nuwro.FillBinFromProjection( proj, cvw )

anl_handler.FinalizeProjection( anl_neut, 1.0 )#  neut_source.fatx()  / neut_source.sumw())
anl_handler.FinalizeProjection( anl_nuwro, 1.0) # nuwro_source.fatx() / nuwro_source.sumw())

plt.errorbar( x=anl_nuwro.x(), y=anl_nuwro.mc(), xerr=0.0, yerr=0.0, label="NUWRO")
plt.errorbar( x=anl_nuwro.x(), y=anl_neut.mc(), xerr=0.0, yerr=0.0, label="NEUT")

plt.xlabel("Q2")
plt.ylabel("Events")
plt.legend()
plt.savefig("test.png")
