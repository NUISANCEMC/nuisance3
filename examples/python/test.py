import pyNUISANCE as pn

evs = pn.EventSource("NUWRO-D2-ANL_77-numu/NUWRO.numu.numu_flux.ANL_1977_2horn_rescan.8652.evts.root")
if not evs:
    print("Error: failed to open input file")

# just get the first event so that we can check the units
ev,_ = evs.first()
ToGeV = 1 if (ev.momentum_unit() == pn.hm.HepMC3.Units.GEV) else 1E-3

def enu_GeV(ev):
    bpart = pn.pps.sel.Beam(ev,14)
    if bpart:
        return bpart.momentum().e()*ToGeV
    return -0

print("Found muon neutrino beam particle with %.2f GeV of energy" % enu_GeV(ev))

fr = pn.FrameGen(evs).limit(int(1000))
fr.add_column("enu_GeV",enu_GeV)
fr.all()

