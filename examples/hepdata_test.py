import pyNUISANCE as pn

mysetup = { "measurement": "T2K_Analysis1",
            "release": "T2K/ThetaMuPMu",
            "table": "CrossSection-CosThetaMuPMu_AnalysisI",
            "input": {"file": "runneut/neutvect_nu.root", "weight": 0.23} }
loader = pn.hepdata.measurement(mysetup)
print(loader.measurement_name)
print(loader.independent_variables)

for v in loader.independent_variables: print(v.summary())

h = loader.createhistogram()
print(h)
print(h.summary())

events = pn.generator.reader( mysetup )



