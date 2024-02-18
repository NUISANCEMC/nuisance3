import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np

# PRO SELECTA TESTS
pn.configure()
pn.ps.LoadFile("../../data/neutrino_data/ANL/CCQE/182176/analysis.cxx")
pn.ps.LoadFile("test.cxx")

print(pn.ps.project.ANL_CCQE_182176_Project_Q2)
print(pn.ps.filter.CUSTOM_FILTER2)

local_func = pn.ps.project.ANL_CCQE_182176_Project_Q2 

# SOURCE TEST
nuwro_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not nuwro_source: sys.exit()

# CUSTOM EVENT LOOP
anlsetup = { "measurement": "ANL_Analysis_mycustomtag",
            "release": "ANL/CCQE/182176/",
            "table": "EventCounts-Q2"}

anl_handler = pn.Measurement(anlsetup)

data = anl_handler.CreateProjection("test")
data.CreateBand("ishighe",2)
data.CreateBand("islowe",3)

original = anl_handler.CreateProjection("original")
original.CreateBand("ishighe",2)
original.CreateBand("islowe",3)

from tqdm import tqdm
 
#[INFO] : Processing Record : 100000it [00:08, 11632.93it/s]
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing Record "):
    # Original fill
    f = anl_handler.FillProjectionFromEvent( original, e )

#[INFO] : Processing PS : 100000it [00:17, 5579.50it/s]
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing PS "):
    # Manual fill of main Q2
    f = pn.ps.filter.ANL_CCQE_182176_Filter(e)
    p = pn.ps.project.ANL_CCQE_182176_Project_Q2(e)
    data.Fill(f, [p], 1.0)

#[INFO] : Processing PRE : 100000it [00:09, 11066.21it/s]
prefilt = pn.ps.filter.ANL_CCQE_182176_Filter
preproj = pn.ps.project.ANL_CCQE_182176_Project_Q2
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing PRE "):
    # Manual fill of main Q2
    f = prefilt(e)
    p = preproj(e)
    data.Fill(f, [p], 1.0)

#[INFO] : Processing Custom : 100000it [00:16, 5909.53it/s]
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing Custom "):
    # Custom fill of secondary bands
    f2 = pn.ps.filter.CUSTOM_FILTER(e)
    data.Fill( f * f2, [p], 1.0)




plt.plot(original.bin_center(), original["mc"].value)
plt.scatter(original.bin_center(), original["mc"].value)
plt.scatter(original.bin_center(), original["data"].value)
plt.savefig("datamc.png")
plt.clf()

plt.plot(data.bin_center(), data["mc"].value)
plt.scatter(data.bin_center(), data["mc"].value)
plt.scatter(data.bin_center(), data["data"].value)
plt.savefig("datamc_new.png")
plt.clf()

plt.plot(data.bin_center(), data["mc"].value)
plt.plot(data.bin_center(), data["ishighe"].value)
plt.plot(data.bin_center(), data["islowe"].value)
plt.savefig("datamc_split.png")
plt.clf()

# OVERRIDING HANDLER CHECK
# anl_handler.projections[0] = custom_Q2

# FRAME GENERATION
def hm_muon(evt):
  part = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kMuon)
  if not part: return 0.0
  return part.momentum().length() * pn.ps.units.GeV

def hm_proton(evt):
  part = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kProton)
  if not part: return 0.0
  return part.momentum().length() * pn.ps.units.GeV

def hm_neutron(evt):
  part = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kNeutron)
  if not part: return 0.0
  return part.momentum().length() * pn.ps.units.GeV

def q0(evt):
    part = pn.ps.parts.q0(evt)
    return part


print("FRAME Evaluation")
fr = pn.FrameGen(nuwro_source, 100000)
fr.AddColumn("Q2", pn.ps.project.ANL_CCQE_182176_Project_Q2)
fr.AddColumn("MeasQ2", anl_handler.projections[0])
fr.AddColumn("custom_filter", pn.ps.filter.CUSTOM_FILTER)
fr.AddColumn("original_filter", pn.ps.filter.ANL_CCQE_182176_Filter)
fr.AddColumn("hm_muon", hm_muon)
fr.AddColumn("hm_proton", hm_proton)
fr.AddColumn("hm_neutron", hm_neutron)
fr.AddColumn("q0", q0)


fr.Limit(99000)
df = fr.Evaluate()

print(df.hm_muon)

plt.hist(df.hm_muon, bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_hmmuon.png")
plt.clf()

plt.scatter(df.hm_muon[df.original_filter > 0], df.hm_proton[df.original_filter > 0], s=2)
plt.savefig("muon_vs_proton.png")
plt.clf()


plt.hist(df.Table[:,2], bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_fromframe.png")
plt.clf()


