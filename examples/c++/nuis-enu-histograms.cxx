#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/Print.h"

#include "boost/histogram.hpp" // make_histogram, regular, weight, indexed

#include "spdlog/spdlog.h"

#include <numeric>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace nuis;
using namespace NuHepMC;
using namespace boost::histogram;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }
  if (!gri) {
    spdlog::critical("Failed to find GenRunInfo for input file {}", argv[1]);
    return 1;
  }

  if (!NuHepMC::GC1::SignalsConvention(gri, "G.C.7")) {
    spdlog::critical("This programme can only handle files that signal NuHepMC "
                     "G.C.7 (Flux distributions)");
    return 1;
  }

  auto procids = NuHepMC::GR4::ReadProcessIdDefinitions(gri);
  auto flux_distribs = NuHepMC::GC7::ReadAllEnergyDistributions(gri);

  auto [ev, w] = evs->first().value();
  auto beampid = NuHepMC::Event::GetBeamParticle(ev)->pid();
  auto tgtpid = NuHepMC::Event::GetBeamParticle(ev)->pid();
  auto tgtA = ((tgtpid / 10) % 1000);

  if (!flux_distribs.count(beampid)) {
    spdlog::critical("A flux distribution with the same beam pid({}) as the "
                     "first event was not stored in the GenRunInfo.",
                     beampid);
    return 1;
  }

  double ToGeV = NuHepMC::Event::ToMeVFactor(ev) * 1E-3;
  double xs_to_pb_N = CrossSection::Units::GetRescaleFactor(
      ev, CrossSection::Units::pb_PerAtom, CrossSection::Units::pb_PerNucleon);

  spdlog::info("xs_to_pb_N: {} -> {} = {}",
               to_string(GC4::ParseCrossSectionUnits(gri)),
               to_string(CrossSection::Units::pb_PerAtom), xs_to_pb_N);

  auto energy_unit = flux_distribs[beampid].energy_unit;
  std::transform(energy_unit.begin(), energy_unit.end(), energy_unit.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (energy_unit == "mev") {
    for (auto &be : flux_distribs[beampid].bin_edges) {
      be *= 1E-3;
    }
  }

  spdlog::info("Histogram energy unit: {}", energy_unit);

  auto rate =
      make_histogram(axis::variable<>(flux_distribs[beampid].bin_edges));
  auto rate_cc =
      make_histogram(axis::variable<>(flux_distribs[beampid].bin_edges));
  auto rate_cc0pi =
      make_histogram(axis::variable<>(flux_distribs[beampid].bin_edges));
  auto rate_cc1pi =
      make_histogram(axis::variable<>(flux_distribs[beampid].bin_edges));

  for (auto const &[ev, cvw] : evs) {

    auto beamp = NuHepMC::Event::GetBeamParticle(ev);

    auto procid = NuHepMC::ER3::ReadProcessID(ev);

    auto allfs = NuHepMC::Event::GetParticles_AllRealFinalState(ev);
    size_t allnuclear =
        std::accumulate(allfs.begin(), allfs.end(), 0, [](int a, auto p) {
          return a + bool(p->pid() > 10000000);
        });
    auto fsmuons = NuHepMC::Event::GetParticles_AllRealFinalState(ev, {13});
    auto fsnucleons =
        NuHepMC::Event::GetParticles_AllRealFinalState(ev, {2212, 2112});
    auto fspions =
        NuHepMC::Event::GetParticles_AllRealFinalState(ev, {211, -211, 111});

    rate(beamp->momentum().e() * ToGeV, weight(cvw));
    if (fsmuons.size()) {
      rate_cc(beamp->momentum().e() * ToGeV, weight(cvw));

      if (fsmuons.size() == 1) {
        if (allfs.size() == (fsmuons.size() + fsnucleons.size() + allnuclear)) {
          rate_cc0pi(beamp->momentum().e() * ToGeV, weight(cvw));
        }
        if (allfs.size() == (fsmuons.size() + fsnucleons.size() +
                             fspions.size() + allnuclear) &&
            (fspions.size() == 1)) {
          rate_cc1pi(beamp->momentum().e() * ToGeV, weight(cvw));
        }
      }
    }
  }

  double fatx = evs->norm_info().fatx;
  double sumw = evs->norm_info().sumweights;

  double sf_cm2_N = (fatx / sumw) * xs_to_pb_N;

  double flux_int = 0;
  for (auto x : flux_distribs[beampid].GetContentCount()) {
    flux_int += x;
  }

  std::stringstream hpydict_bc, hpydict_sig, hpydict_sigccinc, hpydict_sigcc0pi,
      hpydict_sigcc1pi;
  hpydict_bc << "{ \"bin_centers\": [";
  hpydict_sig << " \"sigma_enu\": [";
  hpydict_sigccinc << " \"sigma_enu_ccinc\": [";
  hpydict_sigcc0pi << " \"sigma_enu_cc0pi\": [";
  hpydict_sigcc1pi << " \"sigma_enu_cc1pi\": [";
  for (auto &&x : indexed(rate)) {

    if (!std::isnormal(
            flux_distribs[beampid]
                .GetContentCount()[x.index()])) { // skip bins with no flux
      continue;
    }

    double enu = ((x.bin().lower() + x.bin().upper()) / 2.0);
    double flux_factor =
        flux_distribs[beampid].GetContentCount()[x.index()] / flux_int;
    hpydict_bc << enu << ", ";
    hpydict_sig << ((rate.at(x.index()) / flux_factor) * sf_cm2_N / enu)
                << ", ";
    hpydict_sigccinc << ((rate_cc.at(x.index()) / flux_factor) * sf_cm2_N / enu)
                     << ", ";
    hpydict_sigcc0pi << ((rate_cc0pi.at(x.index()) / flux_factor) * sf_cm2_N /
                         enu)
                     << ", ";
    hpydict_sigcc1pi << ((rate_cc1pi.at(x.index()) / flux_factor) * sf_cm2_N /
                         enu)
                     << ", ";
  }

  hpydict_bc << "],";
  hpydict_sig << "],";
  hpydict_sigccinc << "],";
  hpydict_sigcc0pi << "],";
  hpydict_sigcc1pi << "] }";

  std::cout << hpydict_bc.str().c_str() << std::endl
            << hpydict_sig.str().c_str() << std::endl
            << hpydict_sigccinc.str().c_str() << std::endl
            << hpydict_sigcc0pi.str().c_str() << std::endl
            << hpydict_sigcc1pi.str().c_str() << std::endl;
}