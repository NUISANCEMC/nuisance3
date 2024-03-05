import pyNUISANCE as pn
import time
import matplotlib.pyplot as plt
import numpy as np

# PRO SELECTA TESTS
pn.configure()

neut_source = pn.EventSource("NEUT.numu.enu_nd280_numu.t2kflux_2016_plus250kA.4529.evts.root")
if not neut_source: sys.exit()

weightcalc = pn.WeightCalcFactory().Make(neut_source, {"neut_cardname": "neut.card"})


anl_handler = pn.Measurement(
  { "measurement": "ANL", "release": "ANL/CCQE/182176/", "table": "EventCounts-Q2"}
)
preproj = pn.ps.project.ANL_CCQE_182176_Project_Q2


dictd = {}
dictd["AxlFFCCQE"] = 5
dictd["FAZExp_A1"] = 0
dictd["FAZExp_A2"] = 0
dictd["FAZExp_A3"] = 0
dictd["FAZExp_A4"] = 0

weightcalc.SetParameters(dictd)

for e, cvw in neut_source:
  print(weightcalc(e))
  break

fr = pn.FrameGen(neut_source, 100000)
fr.Limit(100000)
fr.AddColumn("weight", weightcalc)
fr.AddColumn("Q2", preproj)
df = fr.Evaluate()



print(df["weight"])


plt.hist(df["weight"], bins=100)
plt.show()
plt.savefig("weightlist_hist.png")
plt.clf()

# Run multiple evaluations to build event by event splines
weightlist = []
parlist    = []

for val in np.linspace(-3.0,3.0,11):

  dictd["FAZExp_A1"] = val
  print(dictd)
  weightcalc.SetParameters(dictd)

  weightset = fr.Evaluate()
  print(weightset["weight"])

  weightlist.append(weightset["weight"])

  p = np.empty(len(weightset["weight"]))
  p.fill(val)
  parlist.append( p )

  print(weightset["Q2"])
  plt.hist(weightset["Q2"], weights=weightset["weight"], bins=np.linspace(0.0,2.0,100))
  plt.xlim([0.0,2.0])

plt.show()
plt.savefig("weightlist_q2.png")
plt.clf()

plt.scatter( parlist, weightlist, s=2 )
plt.savefig("weightlist.png")
plt.clf()
