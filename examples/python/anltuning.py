import pyNUISANCE as pn
import pyHepMC3 as ph
import sys
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import minimize

pn.env.configure()

# Create Meaurement
anlsetup = { "measurement": "ANL_Analysis_mycustomtag",
            "release": "ANL/CCQE/182176/",
            "table": "EventCounts-Q2",
            "input": {"type": "NEUT", "file": "../../../runs/NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.28328.evts.root", "weight": 1.0} }
anl_handler = pn.hepdata.HEPDataLoader(anlsetup)
anl_data    = anl_handler.CreateRecord("anl_comp")
anl_events  = pn.generator.build_reader(anlsetup)

print("BUILT READER")

print(anl_data.bin_center)
print(anl_data.data_error)

#print("Attempting to calculate a likelihood")
#print(pn.statistical.CalculateRecordLikelihood(anl_data))
#print(anl_data.data_covariance)

print(anl_data.data_value)
print(anl_data.data_error)

plt.errorbar( x=np.array(anl_data.bin_center)[:,0], y=anl_data.data_value, xerr=np.array(anl_data.bin_width)[:,0], yerr=np.sqrt(anl_data.data_value))
plt.savefig("test.png")

#miniboonesetup = { "measurement": "MiniBooNE_CC0pi",
#            "release": "MiniBooNE/CC0pi_XSec_2DPCos_nu",
#            "table": "CrossSection-CosThetaMuPMu_AnalysisI",
#            "input": {"file": "neutvect_nu_miniboone.root", "weight": 1.0} }
#miniboone_handler = pn.hepdata.measurement(miniboonesetup)
#miniboone_data   = miniboone_handler.CreateRecord("miniboone_comp")
#miniboone_events = pn.generator.build_reader(miniboonesetup)

#Create a custom weight handler
# wmanager = pn.reweight.weightmanager()
# wmanager.AddDial("MaCCRES", nominal=0.0, error=1.0, mirror=False, low=-2.0, high=2.0)
# wmanager.AddDial("NormBKG", nominal=0.0, error=1.0, mirror_low=True, mirror_high=False, low=-4.0, high=4.0)
# wmanager.AddNormalisation("MiniBooNE_CC0pi", nominal=1.0, error=0.10)

anl_events.SetDial("MaCCQE", +1.0)

# Standard objective functions will be provided
# that can be used, but support for custom needed
def objective_function(x):

    print("NEW OBJECTIVE : ", x)
    anl_events.SetDial("MaCCQE", x[0])

    anl_handler.FillRecordFromReader(anl_data, anl_events, wmanager)

    chi2_data = (pn.statistical.CalculateRecordLikelihood(anl_data))
    
    return chi2_data

# Now just setup a simple minimizer for now
result = minimize(objective_function, [0.0], "BFGS")
minimized_params = result.x
print(minimized_params)
