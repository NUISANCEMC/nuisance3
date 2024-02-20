
#include "HepMC3/GenEvent.h"

extern "C" {
int CUSTOM_FILTER2(HepMC3::GenEvent const &ev) {
  auto mu = ps::sel::OutPartHM(ev, ps::pdg::kMuon);
  if (!mu) return 0;

  if (mu->momentum().length() > 0.5 * ps::GeV) return 2;
  return 3;
}
}
