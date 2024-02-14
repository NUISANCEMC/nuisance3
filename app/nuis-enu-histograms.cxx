#include "nuis/eventinput/EventSourceFactory.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include <boost/format.hpp>    // only needed for printing
#include <boost/histogram.hpp> // make_histogram, regular, weight, indexed

#include "spdlog/spdlog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace nuis;
using namespace boost::histogram;

int main(int argc, char const *argv[]) {

  EventSourceFactory fact;

  auto [gri, evs] = fact.Make(argv[1]);

  if (!evs) {
    spdlog::critical("Failed to find EventSource for input file {}", argv[1]);
    return 1;
  }

  NuHepMC::StatusCodeDescriptors procids;
  std::map<int, NuHepMC::GC7::EnergyDistribution> flux_distribs;

  if (gri) {
    procids = NuHepMC::GR4::ReadProcessIdDefinitions(gri);
    flux_distribs = NuHepMC::GC7::ReadAllEnergyDistributions(gri);
  }

  if (!flux_distribs.count(14)) {
    spdlog::critical("This application currently only works with numu fluxes.");
    return 1;
  }

  double ToGeV = NuHepMC::Event::ToMeVFactor(evs->first().value().evt) * 1E-3;

  auto energy_unit = flux_distribs[14].energy_unit;
  std::transform(energy_unit.begin(), energy_unit.end(), energy_unit.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (energy_unit == "mev") {
    for (auto &be : flux_distribs[14].bin_edges) {
      be *= 1E-3;
    }
  }

  spdlog::info("Histogram energy unit: {}", energy_unit);

  auto rate = make_histogram(axis::variable<>(flux_distribs[14].bin_edges));
  auto xs = make_profile(axis::variable<>(flux_distribs[14].bin_edges));

  for (auto const &[ev, cvw] : evs) {

    auto beamp = NuHepMC::Event::GetBeamParticle(ev);

    auto procid = NuHepMC::ER3::ReadProcessID(ev);
    auto totxs = NuHepMC::EC2::ReadTotalCrossSection(ev);

    rate(beamp->momentum().e() * ToGeV, weight(cvw));
    xs(beamp->momentum().e() * ToGeV, sample(totxs), weight(cvw));
  }

  double fatx = evs->norm_info().fatx();
  double sumw = evs->norm_info().sumweights();

  double flux_int = 0;
  for (auto x : flux_distribs[14].GetContentCount()) {
    flux_int += x;
  }

  std::stringstream hpydict_bc, hpydict_flux, hpydict_rate, hpydict_sig;
  hpydict_bc << "{ \"bins\": [";
  hpydict_rate << " \"rate\": [";
  hpydict_flux << " \"flux\": [";
  hpydict_sig << " \"sigma_enu\": [";
  for (auto &&x : indexed(rate)) {
    hpydict_bc << " (" << x.bin().lower() << ", " << x.bin().upper() << "), ";
    hpydict_rate << (*x) << ", ";
    hpydict_flux << flux_distribs[14].GetContentCount()[x.index()] << ", ";
    hpydict_sig << ((*x) /
                    (flux_distribs[14].GetContentCount()[x.index()] / flux_int))
                << ", ";
  }
  hpydict_bc << " ],";
  hpydict_rate << " ],";
  hpydict_flux << " ],";
  hpydict_sig << " ] }";

  std::cout << hpydict_bc.str().c_str() << std::endl
            << hpydict_rate.str().c_str() << std::endl
            << hpydict_flux.str().c_str() << std::endl
            << hpydict_sig.str().c_str();
}