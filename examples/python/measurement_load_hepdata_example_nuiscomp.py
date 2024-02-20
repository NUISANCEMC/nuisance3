import numpy as np
import matplotlib.pyplot as plt
import sys
import pyNUISANCE as pn

pn.configure()

def run_comparison(label, source, measurements):

    records = []
    for me in measurements:
        records.append( me.CreateProjection(label) )

    for e, cvw in source:
        for i, me in enumerate(measurements):
            if not me.FilterEvent(e): continue
            proj = me.ProjectEvent(e)
            records[i].FillBinFromProjection( proj, cvw )

    for i, me in enumerate(measurements):
        me.FinalizeProjection( records[i],  source.fatx()  / source.sumw())

        plt.errorbar( x=r.x(), y=r.mc(), xerr=0.0, yerr=0.0, label=label)
        plt.show()

# Dummy example, running multipe measurements at once
measurement_list = [
    pn.Measurement({"measurement": "ANL_Analysis_run1", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}),
    pn.Measurement({"measurement": "ANL_Analysis_run2", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}),
    pn.Measurement({"measurement": "ANL_Analysis_run3", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}),
    pn.Measurement({"measurement": "ANL_Analysis_run4", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}),
    pn.Measurement({"measurement": "ANL_Analysis_run5", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"})
]

run_comparison( 
    "NUWRO",
    pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root"),
    measurement_list
)

run_comparison( 
    "NEUT",
    pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root"),
    measurement_list
)
