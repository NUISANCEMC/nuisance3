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

count = 0
for (e, cvw) in nuwro_source:

    # Original fill
    f = anl_handler.FillProjectionFromEvent( original, e )

    # Manual fill of main Q2
    f = pn.ps.filter.ANL_CCQE_182176_Filter(e)
    p = pn.ps.project.ANL_CCQE_182176_Project_Q2(e)
    data.Fill(f, [p], 1.0)

    # Custom fill of secondary bands
    f2 = pn.ps.filter.CUSTOM_FILTER(e)
    data.Fill( f * f2, [p], 1.0)

    count += 1
    if count % 1000 == 0: print(count)

plt.plot(original.bin_center(), original["mc"].value)
plt.scatter(original.bin_center(), original["mc"].value)
plt.scatter(original.bin_center(), original["data"].value)
plt.savefig("datamc.png")

plt.plot(data.bin_center(), data["mc"].value)
plt.scatter(data.bin_center(), data["mc"].value)
plt.scatter(data.bin_center(), data["data"].value)
plt.savefig("datamc_new.png")

plt.plot(data.bin_center(), data["mc"].value)
plt.plot(data.bin_center(), data["ishighe"].value)
plt.plot(data.bin_center(), data["islowe"].value)
plt.savefig("datamc_split.png")

# OVERRIDING HANDLER CHECK
# anl_handler.projections[0] = custom_Q2

# FRAME GENERATION
def custom_Q2(ev):
    return 0.5

fr = pn.FrameGen(nuwro_source, 50000)
fr.AddColumn("eventproc", custom_Q2)
fr.AddColumn("Q2", pn.ps.project.ANL_CCQE_182176_Project_Q2)
fr.AddColumn("MeasQ2", anl_handler.projections[0])
fr.AddColumn("custom_filter", pn.ps.filter.CUSTOM_FILTER)
fr.AddColumn("original_filter", pn.ps.filter.ANL_CCQE_182176_Filter)

fr.Limit(49999)
df = fr.Evaluate()

print(df.Table)
plt.hist(df.Table[:,2], bins=np.linspace(0.0,2.0,200))
plt.savefig("hist_fromframe.png")

