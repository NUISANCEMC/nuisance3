import pyNUISANCE as pn
import pyProSelecta as pps
import pyNuHepMC as nhm

pps.load_text("""

#include "HepMC3/Print.h"

double enu_GeV(HepMC3::GenEvent const &ev){
  auto bpart = ps::sel::Beam(ev,14);
  if(!bpart) {
      return ps::kMissingDatum<int>;
  }
  return bpart->momentum().e() * 1E-3;
}

int subsample(HepMC3::GenEvent const &ev){
  auto procid = NuHepMC::ER3::ReadProcessID(ev);

  auto bpart = ps::sel::Beam(ev,14);
  if(!bpart) {
      return 0;
  }

  auto cclep = ps::sel::PrimaryCCLepForNu(ev,bpart);
  if(!cclep){
    return 1;
  }

  if( (200 <= procid) && (procid < 400) ){ //qe+2p2h
    return 2;
  } else if ( (400 <= procid) && (procid < 500) ){ //res
    return 3;
  } else if ( (500 <= procid) && (procid < 600) ){ //sis+dis
    return 4;
  }
  return 0;
}
""")

files_low = ["/root/scratch/IOPPaper/ngen_%s/eventsout.low.%s.root" % (x,x) for x in range(10) ]
files_high = ["/root/scratch/IOPPaper/ngen_%s/eventsout.high.%s.root" % (x,x) for x in range(10) ]

evs_low = pn.EventSource({"filepaths":files_low})
if not evs_low:
    print("Error: failed to open input file")

fg_low = pn.EventFrameGen(evs_low,int(1E6)).limit(1E7) \
    .add_column("enu_GeV", pps.project.get("enu_GeV")) \
    .add_column("sample", pps.select.get("subsample"))

enu_hist = pn.HistFrame(pn.Binning.lin_space(0,10,50))
enu_hist.add_column("NC")
enu_hist.add_column("QE+MEC")
enu_hist.add_column("RES")
enu_hist.add_column("DIS")

chunk = fg_low.firstArrow()
while chunk:
    enu_hist.fill_columns_from_RecordBatch(chunk, ["enu_GeV"], "sample")
    print("filled %s rows." % chunk.num_rows)
    chunk = fg_low.nextArrow()

import numpy as np
stashw = enu_hist.sumweights.copy()
stashv = enu_hist.variances.copy()