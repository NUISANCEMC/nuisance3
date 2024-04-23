import pyNUISANCE as pn
import numpy as np
import pyProSelecta as pps
import pyNuHepMC as nhm
import matplotlib.pyplot as plt
import scipy
import yaml

# Fetch the submission YAML
submission_path = "/root/software/NUISANCEMC/data/nu/MicroBooNE/Ar/numu/arxiv.2310.06082/testout"
submission = "submission.yaml"

data_files = {}

with open("/".join([submission_path, submission]), 'r') as file:
    submission_yaml = yaml.safe_load_all(file)
    for doc in submission_yaml:
        if "data_file" in doc:
            df = doc["data_file"]
            with open("/".join([submission_path, df]), 'r') as datafile:
                data_files[df] = yaml.safe_load(datafile)
                
print(data_files.keys())

measurements = []

for var in ["pn","alpha3d","phi3d"]:
    meas = {}
    meas["doc"] = data_files["cross_section-%s.yaml" %var]
    meas["indep"] = [ kv["header"]["name"] for kv in meas["doc"]["independent_variables"] ]
    meas["quals"] = { kv["name"]:kv["value"] \
        for kv in meas["doc"]["dependent_variables"][0]["qualifiers"]}
    
    print("Measurement: %s" % var)
    print("  Target: %s" % meas["quals"]["target"])
    print("  Probe Species: %s" % meas["quals"]["probe_species"])
    print("  Independent variables: %s" % meas["indep"])
    print("  Projection functions: %s" % \
          [ meas["quals"]["project:%s" % x] for x in meas["indep"] ])
    print("  Selection function: %s" %  meas["quals"]["select"])

    measurements.append(meas)

evs = pn.EventSource("prdgen/MicroBooNE.nuwro.eventsout.root")
if not evs:
    print("Error: failed to open input file")

#load the select/project functions
pps.load_file("/".join([submission_path, "../analysis.cxx"]))

fg = pn.EventFrameGen(evs).filter(pps.select.get(measurements[0]["quals"]["select"]))
fg.add_column("enu",pps.project.enu)
for m in measurements:
    fg.add_column(m["indep"][0],pps.project.get(m["quals"]["project:%s" % m["indep"][0]]))
    
fg.add_column("MicroBooNE_CC0Pi_nu_dpt",pps.project.get("MicroBooNE_CC0Pi_nu_dpt"))
fg.add_column("MicroBooNE_CC0Pi_nu_dphit",pps.project.get("MicroBooNE_CC0Pi_nu_dphit"))
fg.add_column("MicroBooNE_CC0Pi_nu_dat",pps.project.get("MicroBooNE_CC0Pi_nu_dat"))

ef = fg.firstArrow(int(1E5))

enuh = pn.HistFrame(pn.Binning.lin_space(0,5000,100))
enuh.fill_procid_columns_from_Arrow(ef, ["enu"])
enuh.mpl().hist(histtype="step")
plt.show()