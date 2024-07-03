import pyNUISANCE as pn
import numpy as np
import pyProSelecta as pps

pn.log.set_level("EventInput",pn.log.level.trace)
evs = pn.EventSource("neutvect.t2kflux_numu_C_nofsi.root")
if not evs:
    print("Error: failed to open input file")