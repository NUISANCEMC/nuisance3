import pyNUISANCE as pn
import numpy as np
import pyProSelecta as pps
import pyNuHepMC as nhm
import matplotlib.pyplot as plt
import scipy

T2K_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/t2kflux_2016_plus250kA.root",
    "enu_sk_numu", "GEV", True)

print("T2K_Flux: {energy_unit: %s, rate_unit: %s}" % (T2K_Flux.energy_unit,T2K_Flux.rate_unit))

NOvA_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/FHC_Flux_NOvA_ND_2017.root",
    "flux_numu", "GEV", True)

print("NOvA_Flux: {energy_unit: %s, rate_unit: %s}" % (NOvA_Flux.energy_unit,NOvA_Flux.rate_unit))

ub_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/BNB_uBooNE_numu_flux_2019.root",
    "numu", "GEV", True)

print("ub_Flux: {energy_unit: %s, rate_unit: %s}" % (ub_Flux.energy_unit,ub_Flux.rate_unit))

DUNE_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/flux_dune_neutrino_ND.root",
    "numu_flux", "GEV", False)

print("DUNE_Flux: {energy_unit: %s, rate_unit: %s}" % (DUNE_Flux.energy_unit, DUNE_Flux.rate_unit))

MNVLE_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/minerva_le_flux.root",
    "numu_fhc", "GEV", True)

print("MNVLE_Flux: {energy_unit: %s, rate_unit: %s}" % (MNVLE_Flux.energy_unit,MNVLE_Flux.rate_unit))

MNVME_Flux = pn.convert.ROOT.get_EnergyDistribution_from_ROOT(\
    "/root/software/NUISANCEMC/nuisance/build/Linux/data/flux/MINERvA_ME_Flux_No_Constraint.root",
    "reweightedflux_rebinned_CV_WithStatErr", "GEV", True)

print("MNVME_Flux: {energy_unit: %s, rate_unit: %s}" % (MNVME_Flux.energy_unit,MNVME_Flux.rate_unit))

pps.load_text("""

double enu_GeV(HepMC3::GenEvent const &ev){
  auto bpart = ps::sel::Beam(ev,14);
  if(!bpart) {
      return ps::kMissingDatum<int>;
  }
  return bpart->momentum().e() * 1E-3;
}

int sample_topo(HepMC3::GenEvent const &ev){
  auto procid = NuHepMC::ER3::ReadProcessID(ev);

  auto bpart = ps::sel::Beam(ev,14);
  if(!bpart) {
      return 0;
  }

  auto cclep = ps::sel::PrimaryCCLepForNu(ev,bpart);
  if(!cclep){
    return 1;
  }

  auto nparts = ps::sel::OutPartsAny(ev,{}).size();
  auto weird_stuff = ps::sel::OutPartsExceptAny(ev, {13,2112,2212,111,211,-211,22});
  auto nleps = ps::sel::OutPartsAny(ev,{13,-13}).size();
  auto npi0 = ps::sel::OutParts(ev,111).size();
  auto ncpi = ps::sel::OutPartsAny(ev,{211,-211}).size();
  auto gammas = ps::sel::OutParts(ev,22);

  for(auto &p : weird_stuff){
    if((p->pid() < 1000000000)){
      return 2; // other
    }
  }

  for(auto &p : gammas){
    if(p->momentum().e() > 25){
      return 2;//other
    }
  }

  if((nleps != 1) || (npi0 != 0) || (ncpi > 1)){
    return 2; //other
  }
  if(ncpi == 1){
    return 3; // 1cpi
  }
  return 4; //0pi
}

int sample_mode(HepMC3::GenEvent const &ev){
  auto procid = NuHepMC::ER3::ReadProcessID(ev);

  auto bpart = ps::sel::Beam(ev,14);
  if(!bpart) {
      return 0;
  }

  auto cclep = ps::sel::PrimaryCCLepForNu(ev,bpart);
  if(!cclep){
    return 1;
  }

  if( (200 <= procid) && (procid < 300) ){ //qe
    return 2;
  } else if( (300 <= procid) && (procid < 400) ){ //2p2h
    return 3;
  } else if ( (400 <= procid) && (procid < 500) ){ //res
    return 4;
  } else if ( (500 <= procid) && (procid < 700) ){ //sis+dis
    return 5;
  }
  return 6;
}

""")

files_low = ["/root/scratch/IOPPaper/gengen_low_%s/ghep.low.%s.root" % (x,x) for x in range(10) ]
files_high = ["/root/scratch/IOPPaper/gengen_high_%s/ghep.high.%s.root" % (x,x) for x in range(10) ]

evs_low = pn.EventSource({
    "filepaths":files_low,
    "spline_file": "/root/scratch/IOPPaper/IOP_review_scripts/generation/MC_inputs/G18_10a_00_000_v320_splines.xml.gz",
    "tune": "G18_10a_00_000"
    })
if not evs_low:
    print("Error: failed to open input file")

evs_high = pn.EventSource({
    "filepaths":files_high,
    "spline_file": "/root/scratch/IOPPaper/IOP_review_scripts/generation/MC_inputs/G18_10a_00_000_v320_splines.xml.gz",
    "tune": "G18_10a_00_000"
    })
if not evs_high:
    print("Error: failed to open input file")