import pyNUISANCE as pn
import numpy as np
import pyProSelecta as pps
import pyNuHepMC as nhm
import matplotlib.pyplot as plt
import scipy
import yaml

pn.log.set_level("EventInput",pn.log.level.debug)


evs_on_axis = pn.EventSource("prdgen/onaxis.CH.neutvect.root")
if not evs_on_axis:
    print("Error: failed to open input file")

evs_off_axis = pn.EventSource("prdgen/offaxis.CH.neutvect.root")
if not evs_off_axis:
    print("Error: failed to open input file")
