import pyEventInput as ev
import sys

evs = ev.EventSource(sys.argv[1])

if not evs:
  print("Failed to acquire event source for %s" % sys.argv[1])
  exit(1)

for i, (ev, cvw) in enumerate(evs):
  if i and (((i+1)% 50000) == 0):
    print("On event %s, CV weight = %s, FATX best estimate = %s pb, sum CV weights = %s " \
      % (i, cvw, evs.fatx(), evs.sumw()))