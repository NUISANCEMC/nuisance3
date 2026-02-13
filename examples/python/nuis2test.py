import pyNUISANCE as pn

rfact = pn.RecordFactory()

records = []
tables = []

measurements = [
  ("DeltaPT_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaPT_nu"),
  ("DeltaAlphaT_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaAlphaT_nu"),
  ("DeltaPhiT_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaPhiT_nu"),
  ("MuonCosTheta_nu","MicroBooNE_CC1Mu1p_XSec_1DMuonCosTheta_nu"),
  ("ProtonCosTheta_nu","MicroBooNE_CC1Mu1p_XSec_1DProtonCosTheta_nu"),
  ("MuonMomentum_nu","MicroBooNE_CC1Mu1p_XSec_1DMuonMomentum_nu"),
  ("DeltaPn_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaPn_nu"),
  ("DeltaPtx_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaPtx_nu"),
  ("DeltaPty_nu","MicroBooNE_CC1Mu1p_XSec_1DDeltaPty_nu"),
  ("ECal_nu","MicroBooNE_CC1Mu1p_XSec_1DECal_nu")
]

for ptag, mname in measurements:
  records.append(rfact.make({"type": "nuisance2", "name":mname}))
  tables.append((ptag, records[-1].table("")))

evs = pn.EventSource("dune_argon_sf_10mega.nuwro.pb.gz")
if not evs:
    print("Error: failed to open input file")

fg = pn.FrameGen(evs).limit(1E5)

fg.filter(tables[0][1].select)

for ptag, tbl in tables:
  fg.add_double_columns([ptag,], tbl.project)

print(fg.all())