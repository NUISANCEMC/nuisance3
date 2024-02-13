import pyEventInput as ev

import sys

inpev = ev.EventSource(sys.argv[1])

for ev, cvw in inpev:
  if ev.event_number() and ((ev.event_number() % 50000) == 0):
    print("On event %s, CV weight = %s, FATX best estimate = %s " % (ev.event_number(), cvw, inpev.fatx()))