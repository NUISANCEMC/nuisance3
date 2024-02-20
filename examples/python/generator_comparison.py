import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np

# PRO SELECTA TESTS
pn.configure()
pn.ps.LoadFile("../../data/neutrino_data/ANL/CCQE/182176/analysis.cxx")

nuwro_source = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not nuwro_source: sys.exit()

neut_source = pn.EventSource("NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root")
if not neut_source: sys.exit()

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
    mu = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kMuon)
    nu = pn.ps.sel.Beam(evt, pn.ps.pdg.kNuMu)
    part = pn.ps.proj.parts.q0(nu,mu)
    if not mu or not nu: return 0
    return part

def q3(evt):
    mu = pn.ps.sel.OutPartHM(evt, pn.ps.pdg.kMuon)
    nu = pn.ps.sel.Beam(evt, pn.ps.pdg.kNuMu)
    part = pn.ps.proj.parts.q3(nu,mu)
    if not mu or not nu: return 0
    return part

def frame_plot_creator(name, source):
    
  global count
  count = 0
  def global_print(ev):
    global count
    count += 1
    if (count % 1000 == 0): print(count)
    return count

  fr = pn.FrameGen(source, 100000)
  fr.AddColumn("id", global_print)
  fr.AddColumn("Q2", pn.ps.project.ANL_CCQE_182176_Project_Q2)
  fr.AddColumn("custom_filter", pn.ps.filter.CUSTOM_FILTER)
  fr.AddColumn("original_filter", pn.ps.filter.ANL_CCQE_182176_Filter)
  fr.AddColumn("hm_muon", hm_muon)
  fr.AddColumn("hm_proton", hm_proton)
  fr.AddColumn("hm_neutron", hm_neutron)
  fr.AddColumn("q0", q0)
  fr.AddColumn("q3", q3)

  fr.Limit(99999)
  df = fr.Evaluate()

  plt.hist(df.hm_muon, bins=np.linspace(0.0,2.0,200))
  plt.savefig(f"hist_hmmuon_{name}.png")
  plt.clf()

  plt.scatter(df.hm_muon[df.original_filter > 0], df.hm_proton[df.original_filter > 0], s=2)
  plt.savefig(f"muon_vs_proton_{name}.png")
  plt.clf()

  plt.scatter(df.q3, df.q0, s=2)
  plt.scatter(df.q3[df.original_filter > 0], df.q0[df.original_filter > 0], s=2)
  plt.savefig(f"q0_vs_q3_{name}.png")
  plt.clf()

  plt.hist(df.Table[:,2], bins=np.linspace(0.0,2.0,200))
  plt.savefig(f"hist_fromframe_{name}.png")
  plt.clf()

  return df
