import pyNUISANCE as pn
import pyHepMC3 as ph
import sys
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import minimize

pn.env.configure()

# Create Meaurement
t2ksetup = { "measurement": "T2K_Analysis_mycustomtag",
            "release": "T2K/CC0pi_XSec_2DPcos_nu",
            "table": "CrossSection-CosThetaMuPMu_AnalysisI",
            "input": {"file": "neutvect_nu.root", "weight": 1.0} }
t2k_handler = pn.hepdata.HEPDataRecord(t2ksetup)
t2k_data    = t2k_handler.CreateProjection("t2k_comp")
t2k_events  = pn.generator.build_reader(t2ksetup)

print("Attempting to calculate a likelihood")
print(pn.statistical.CalculateProjectionLikelihood(t2k_data))
print(t2k_data.data_covariance)

#miniboonesetup = { "measurement": "MiniBooNE_CC0pi",
#            "release": "MiniBooNE/CC0pi_XSec_2DPCos_nu",
#            "table": "CrossSection-CosThetaMuPMu_AnalysisI",
#            "input": {"file": "neutvect_nu_miniboone.root", "weight": 1.0} }
#miniboone_handler = pn.hepdata.measurement(miniboonesetup)
#miniboone_data   = miniboone_handler.CreateProjection("miniboone_comp")
#miniboone_events = pn.generator.build_reader(miniboonesetup)

#Create a custom weight handler
wmanager = pn.reweight.weightmanager()
wmanager.AddDial("MaCCRES", nominal=0.0, error=1.0, mirror=False, low=-2.0, high=2.0)
wmanager.AddDial("NormBKG", nominal=0.0, error=1.0, mirror_low=True, mirror_high=False, low=-4.0, high=4.0)
wmanager.AddNormalisation("MiniBooNE_CC0pi", nominal=1.0, error=0.10)

# Standard objective functions will be provided
# that can be used, but support for custom needed
def objective_function(x):

    print("NEW OBJECTIVE : ", x)
    wmanager.SetDials(x)

    t2k_handler.FillProjectionFromReader(t2k_data, t2k_events, wmanager)
    miniboone_handler.FillProjectionFromReader(miniboone_data, miniboone_events, wmanager)

    chi2_data = (pn.statistical.CalculateProjectionLikelihood(t2k_data) +
        pn.statistical.CalculateProjectionLikelihood(miniboone_data))
    
    chi2_dial = pn.statistical.CalculateDialLikelihood(wmanager)
    chi2_norm = pn.statistical.CalclulateNormLikelihood(wmanager)

    return chi2_data + chi2_data + chi2_norm

# Now just setup a simple minimizer for now
result = minimize(objective_function, wmanager.GetNominals(), "BFGS")
minimized_params = result.x
print(minimized_params)
