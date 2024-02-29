import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np

# PRO SELECTA TESTS
pn.configure()

# SOURCE TEST
nuwro_source = pn.EventSource("test2_kaskada.root")
if not nuwro_source: sys.exit()

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

for evt, cw in nuwro_source:
  for vertex in evt.vertices():
      if vertex.id() == -3:
        print("VERT POS", vertex.position().x())

  break

# Do some testing to grab what we want
for evt, cw in nuwro_source:
    outp = pn.ps.sel.OutPartHM(evt,int(pn.ps.pdg.kNeutron))
    inp = pn.ps.sel.Beam(evt, int(pn.ps.pdg.kNeutron))

    print("INP", outp, inp)

    for vertex in evt.vertices():
      if vertex.id() == -3: # Nucleon seperation vertex before FSI to get incoming in kaskada
          for part in vertex.particles_in():
             print("VERTEX -3, IS_IN", part.status(), part.pid(), part.momentum().e() - part.momentum().m())
             if part.pid() == 2112:
                inp = part
      if vertex.id() == -2:
          for part in vertex.particles_in():
             print("FSI", part.status(), part.pid(), part.momentum().e() - part.momentum().m())
      if vertex.id() == -1: # Hard Scattering outgoing particles
          for part in vertex.particles_out():
             print("VERTEX -1, IS_OUT", part.status(), part.pid(), part.momentum().e() - part.momentum().m())
             if part.pid() == 2112:
                outp = part
          
    if not outp or not inp: continue
    break


    
# Put all that in a function instead and evaluate into a frame
def find_scatter_difference(evt):
  inp = None
  outp = None
  for vertex in evt.vertices():
    if vertex.id() == -3: # Nucleon seperation vertex before FSI to get incoming in kaskada
        for part in vertex.particles_in():
            if part.pid() == 2112:
              inp = part
    if vertex.id() == -1: # Hard Scattering outgoing particles
        for part in vertex.particles_out():
            if part.pid() == 2112:
              outp = part
        
  if not outp or not inp: return -1.0
  return (inp.momentum() - outp.momentum()).e()

print("FRAME Evaluation")
fr = pn.FrameGen(nuwro_source, 100000)
fr.AddColumn("hm_muon", hm_muon)
fr.AddColumn("hm_proton", hm_proton)
fr.AddColumn("hm_neutron", hm_neutron)
fr.AddColumn("scatter_dif", find_scatter_difference)

fr.Limit(99000)
df = fr.Evaluate()

print(df["scatter_dif"])
plt.scatter(df["scatter_dif"], df["hm_proton"], s=2)
plt.savefig("scatterdif.png")
plt.clf()

plt.hist( df.hm_proton[ df.scatter_dif < 0.001] )
plt.savefig("scatteramp.png")
plt.clf()
