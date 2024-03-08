import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np
import pyProSelecta as pps
# PRO SELECTA TESTS
# pn.configure()
pps.load_file("../../data/neutrino_data/ANL/CCQE/182176/analysis.cxx")

print(pps.project.ANL_CCQE_182176_Project_Q2)
print(pps.filter.CUSTOM_FILTER2)

local_func = pps.project.ANL_CCQE_182176_Project_Q2 

# SOURCE TEST
nuwro_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not nuwro_source: sys.exit()

neut_source = pn.EventSource("NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root")
if not neut_source: sys.exit()

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
    f = pps.filter.ANL_CCQE_182176_Filter(e)
    p = pps.project.ANL_CCQE_182176_Project_Q2(e)
    data.Fill(f, [p], 1.0)

#[INFO] : Processing PRE : 100000it [00:09, 11066.21it/s]
prefilt = pps.filter.ANL_CCQE_182176_Filter
preproj = pps.project.ANL_CCQE_182176_Project_Q2
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing PRE "):
    # Manual fill of main Q2
    f = prefilt(e)
    p = preproj(e)
    data.Fill(f, [p], 1.0)

#[INFO] : Processing Custom : 100000it [00:16, 5909.53it/s]
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing Custom "):
    # Custom fill of secondary bands
    f2 = pps.filter.CUSTOM_FILTER(e)
    data.Fill( f * f2, [p], 1.0)


def hm_muon(evt):
  part = pps.sel.OutPartHM(evt, pps.pdg.kMuon)
  if not part: return 0.0
  return part.momentum().length() * pps.units.GeV
#[INFO] : Processing Custom PY : 100000it [00:16, 6125.59it/s]
for (e, cvw) in tqdm (nuwro_source, desc="[INFO] : Processing Custom PY "):
    # Custom fill of secondary bands
    p2 = hm_muon(e)
    f2 = pps.filter.CUSTOM_FILTER(e)
    data.Fill( f * f2, [p2], 1.0)



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
  part = pps.sel.OutPartHM(evt, pps.pdg.kMuon)
  if not part: return 0.0
  return part.momentum().length() * pps.units.GeV

def hm_proton(evt):
  part = pps.sel.OutPartHM(evt, pps.pdg.kProton)
  if not part: return 0.0
  return part.momentum().length() * pps.units.GeV

def hm_neutron(evt):
  part = pps.sel.OutPartHM(evt, pps.pdg.kNeutron)
  if not part: return 0.0
  return part.momentum().length() * pps.units.GeV

def q0(evt):
    mu = pps.sel.OutPartHM(evt, pps.pdg.kMuon)
    nu = pps.sel.Beam(evt, pps.pdg.kNuMu)
    part = pps.proj.parts.q0(nu,mu)
    if not mu or not nu: return 0
    return part

def q3(evt):
    mu = pps.sel.OutPartHM(evt, pps.pdg.kMuon)
    nu = pps.sel.Beam(evt, pps.pdg.kNuMu)
    part = pps.proj.parts.q3(nu,mu)
    if not mu or not nu: return 0
    return part



print("FRAME Evaluation")
fr = pn.FrameGen(nuwro_source, 100000)
fr.AddColumn("Q2", pps.project.ANL_CCQE_182176_Project_Q2)
fr.AddColumn("MeasQ2", anl_handler.projections[0])
fr.AddColumn("custom_filter", pps.filter.CUSTOM_FILTER)
fr.AddColumn("original_filter", pps.filter.ANL_CCQE_182176_Filter)
fr.AddColumn("hm_muon", hm_muon)
fr.AddColumn("hm_proton", hm_proton)
fr.AddColumn("hm_neutron", hm_neutron)
fr.AddColumn("q0", q0)
fr.AddColumn("q3", q3)

fr.Limit(99000)
df = fr.Evaluate()

print(df.hm_muon)

plt.hist(df.hm_muon, bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_hmmuon.png")
plt.clf()

plt.scatter(df.hm_muon[df.original_filter > 0], df.hm_proton[df.original_filter > 0], s=2)
plt.savefig("muon_vs_proton.png")
plt.clf()

plt.scatter(df.q3, df.q0, s=2)
plt.scatter(df.q3[df.original_filter > 0], df.q0[df.original_filter > 0], s=2)
plt.savefig("q0_vs_q3.png")
plt.clf()

plt.hist(df.Table[:,2], bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_fromframe.png")
plt.clf()



print("FRAME Evaluation")
fr = pn.FrameGen(neut_source, 100000)
fr.AddColumn("Q2", pps.project.ANL_CCQE_182176_Project_Q2)
fr.AddColumn("MeasQ2", anl_handler.projections[0])
fr.AddColumn("custom_filter", pps.filter.CUSTOM_FILTER)
fr.AddColumn("original_filter", pps.filter.ANL_CCQE_182176_Filter)
fr.AddColumn("hm_muon", hm_muon)
fr.AddColumn("hm_proton", hm_proton)
fr.AddColumn("hm_neutron", hm_neutron)
fr.AddColumn("q0", q0)
fr.AddColumn("q3", q3)

fr.Limit(99000)
df = fr.Evaluate()

print(df.hm_muon)

plt.hist(df.hm_muon, bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_hmmuon_neut.png")
plt.clf()

plt.scatter(df.hm_muon[df.original_filter > 0], df.hm_proton[df.original_filter > 0], s=2)
plt.savefig("muon_vs_proton_neut.png")
plt.clf()

plt.scatter(df.q3, df.q0, s=2)
plt.scatter(df.q3[df.original_filter > 0], df.q0[df.original_filter > 0], s=2)
plt.savefig("q0_vs_q3_neut_neut.png")
plt.clf()

plt.hist(df.Table[:,2], bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_fromframe_neut.png")
plt.clf()


