import pyEVENTINPUT as ev
import pyMEASUREMENT as me
import numpy as np
import matplotlib.pyplot as plt
from pyHepMC3 import HepMC3 as hm

me.env.configure()

nuwro_source = ev.EventSource("./nuwrofile.root")
neut_source = ev.EventSource("../../runs/NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.9247.evts.root")

anlsetup = { "measurement": "ANL_Analysis_mycustomtag",
            "release": "ANL/CCQE/182176/",
            "table": "EventCounts-Q2"}

anl_handler = me.measurement.HEPDataLoader(anlsetup)
anl_nuwro = anl_handler.CreateRecord("anl_comp_nuwro")
anl_neut  = anl_handler.CreateRecord("anl_comp_neut")

for e in neut_source:
  anl_handler.FillRecordFromEvent( anl_neut, e, 1.0 )

for e in nuwro_source:
  anl_handler.FillRecordFromEvent( anl_nuwro, e, 1.0 )

plt.errorbar( x=np.array(anl_nuwro.bin_center)[:,0], y=anl_nuwro.mc_weights, xerr=np.array(anl_nuwro.bin_width)[:,0], yerr=0.0, label="NUWRO")
plt.errorbar( x=np.array(anl_neut.bin_center)[:,0], y=anl_neut.mc_weights, xerr=np.array(anl_neut.bin_width)[:,0], yerr=0.0, label="NEUT")
plt.xlabel("Q2")
plt.ylabel("Events")
plt.legend()
plt.savefig("test.png")
