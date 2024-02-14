import pyNUISANCE as nuis

evs = nuis.EventSource("../../data/test_event_samples/nuwro-sample-ANL.root")

if not evs.good():
  print("Failed to open input file")
  exit(1)

for ev, _ in evs:
  print(ev)
  # Do something with each event, ev, which are HepMC3::GenEvents