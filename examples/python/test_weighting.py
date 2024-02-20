import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np

# PRO SELECTA TESTS
pn.configure()

neut_source = pn.EventSource("NEUT-D2-ANL_77-numu/NEUT.numu.numu_flux.ANL_1977_2horn_rescan.23500.evts.root")
if not neut_source: sys.exit()

weightcalc = pn.WeightCalcFactory().Make(neut_source, {"neut_cardname": "neut.card"})

weightcalc.SetParameters(
  {"MaCCQE": -2}
)

for e, cvw in neut_source:
  print(weightcalc(e))
  break

fr = pn.FrameGen(neut_source, 100000)
fr.Limit(100000)
fr.AddColumn("weight", weightcalc)
df = fr.Evaluate()

print(df["weight"])

# plt.hist(df["weight"])
# plt.show()
# plt.clf


# Run multiple evaluations to build event by event splines
weightlist = []
parlist    = []
for val in np.linspace(-3.0,3.0,11):

  weightcalc.SetParameters(
    {"MaCCQE": val}
  )

  weightset = fr.Evaluate()["weight"]
  weightlist.append(weightset)

  p = np.empty(len(weightset))
  p.fill(val)
  parlist.append( p )
  

plt.scatter( parlist, weightlist, s=2 )
plt.savefig("weightlist.png")
plt.clf()
