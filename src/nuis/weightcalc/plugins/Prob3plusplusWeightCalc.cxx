#include "nuis/weightcalc/plugins/Prob3plusplusWeightCalc.h"

#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "NuHepMC/EventUtils.hxx"

#include "HepMC3/GenParticle.h"

#include "BargerPropagator.h"

#include "boost/dll/alias.hpp"

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

namespace {
constexpr double deg2rad = asin(1) / 90.0;
constexpr double REarth_km = 6371.393;
} // namespace

Prob3plusplusWeightCalc::NuTypes GetNuType(int pdg) {
  switch (pdg) {
  case 16:
    return Prob3plusplusWeightCalc::kNutauType;
  case 14:
    return Prob3plusplusWeightCalc::kNumuType;
  case 12:
    return Prob3plusplusWeightCalc::kNueType;
  case -16:
    return Prob3plusplusWeightCalc::kNutaubarType;
  case -14:
    return Prob3plusplusWeightCalc::kNumubarType;
  case -12:
    return Prob3plusplusWeightCalc::kNuebarType;
  default: {
    log_warn("[Prob3plusplusWeightCalc]: Invalid neutrino PDG code: {}",
                 pdg);
    return Prob3plusplusWeightCalc::kInvalid;
  }
  }
}

Prob3plusplusWeightCalc::Prob3plusplusWeightCalc(YAML::Node const &) {
  prop = std::make_unique<BargerPropagator>();
}

Prob3plusplusWeightCalc::~Prob3plusplusWeightCalc() {}

void Prob3plusplusWeightCalc::set_dipangle(double dip_angle_deg) {
  LengthParam = std::cos((90.0 + dip_angle_deg) * deg2rad);
}
void Prob3plusplusWeightCalc::set_baseline(double baseline_km) {
  set_dipangle(std::asin(baseline_km / (2.0 * REarth_km)) / deg2rad);
}

double Prob3plusplusWeightCalc::prob(double enu_GeV) {

  if (from_type == kInvalid) {
    log_warn("[Prob3plusplusWeightCalc]: Oscillation channel not "
                 "configured: \"osc:from\" == kInvalid");
    return 1;
  }

  if (to_type == kInvalid) {
    log_warn("[Prob3plusplusWeightCalc]: Oscillation channel not "
                 "configured: \"osc:to\" == kInvalid");
    return 1;
  }

  prop->SetMNS(sinsq_th12, sinsq_th13, sinsq_th23, dmsq_21, dmsq_atm, dcp_rad,
               enu_GeV, true, from_type);
  prop->DefinePath(LengthParam, 0);
  prop->propagate(to_type);

  return prop->GetProb(from_type, to_type);
}

double Prob3plusplusWeightCalc::calc_weight(HepMC3::GenEvent const &ev) {
  auto beamp = NuHepMC::Event::GetBeamParticle(ev);
  if (!beamp) {
    log_warn("[Prob3plusplusWeightCalc]: Failed to find valid beam "
                 "particle in event");
    return 1;
  }
  if (from_type == kInvalid) {
    from_type = GetNuType(beamp->pid());
  }
  if (to_type == kInvalid) {
    to_type = GetNuType(beamp->pid());
  }

  return prob(beamp->momentum().e() * NuHepMC::Event::ToMeVFactor(ev) * 1E-3);
}

void Prob3plusplusWeightCalc::set_parameters(
    std::map<std::string, double> const &params) {

  if (params.count("sinsq_th12")) {
    sinsq_th12 = params.at("sinsq_th12");
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th12 = {}", sinsq_th12);
  }
  if (params.count("th12")) {
    sinsq_th12 = std::pow(std::sin(params.at("th12")), 2);
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th12 = {}", sinsq_th12);
  }

  if (params.count("sinsq_th13")) {
    sinsq_th13 = params.at("sinsq_th13");
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th13 = {}", sinsq_th13);
  }
  if (params.count("th13")) {
    sinsq_th13 = std::pow(std::sin(params.at("th13")), 2);
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th13 = {}", sinsq_th13);
  }

  if (params.count("sinsq_th23")) {
    sinsq_th23 = params.at("sinsq_th23");
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th23 = {}", sinsq_th23);
  }
  if (params.count("th23")) {
    sinsq_th23 = std::pow(std::sin(params.at("th23")), 2);
    log_info("[Prob3plusplusWeightCalc]: Set sinsq_th23 = {}", sinsq_th23);
  }

  if (params.count("dmsq_21")) {
    dmsq_21 = params.at("dmsq_21");
    log_info("[Prob3plusplusWeightCalc]: Set dmsq_21 = {}", dmsq_21);
  }
  if (params.count("dmsq_atm")) {
    dmsq_atm = params.at("dmsq_atm");
    log_info("[Prob3plusplusWeightCalc]: Set dmsq_atm = {}", dmsq_atm);
  }

  if (params.count("dcp_rad")) {
    dcp_rad = params.at("dcp_rad");
    log_info("[Prob3plusplusWeightCalc]: Set dcp_rad = {}", dcp_rad);
  }
  if (params.count("dcp_npi")) {
    dcp_rad = params.at("dcp_npi") * M_PI;
    log_info(
        "[Prob3plusplusWeightCalc]: Set dcp_rad = {}, from dcp_npi = {}",
        dcp_rad, params.at("dcp_npi"));
  }

  if (params.count("baseline_km")) {
    set_baseline(params.at("baseline_km"));
    log_info("[Prob3plusplusWeightCalc]: Set baseline_km = {}",
                 params.at("baseline_km"));
  }

  if (params.count("dip_angle_deg")) {
    set_dipangle(params.at("dip_angle_deg"));
    log_info("[Prob3plusplusWeightCalc]: Set dip_angle_deg = {}",
                 params.at("dip_angle_deg"));
  }

  if (params.count("osc:from")) {
    from_type = GetNuType(params.at("osc:from"));
    log_info("[Prob3plusplusWeightCalc]: Set oscillation from {}",
                 from_type);
  }

  if (params.count("osc:to")) {
    to_type = GetNuType(params.at("osc:to"));
    log_info("[Prob3plusplusWeightCalc]: Set oscillation to {}", to_type);
  }

  if (params.count("osc:numu_disp")) {
    from_type = GetNuType(14);
    to_type = GetNuType(14);
    log_info("[Prob3plusplusWeightCalc]: Set numu disappearance "
                 "oscillation channel: {} -> {}",
                 from_type, to_type);
  }

  if (params.count("osc:numubar_disp")) {
    from_type = GetNuType(-14);
    to_type = GetNuType(-14);
    log_info("[Prob3plusplusWeightCalc]: Set numubar disappearance "
                 "oscillation channel: {} -> {}",
                 from_type, to_type);
  }

  if (params.count("osc:numu_to_nue")) {
    from_type = GetNuType(14);
    to_type = GetNuType(12);
    log_info("[Prob3plusplusWeightCalc]: Set numu to nue appearance "
                 "oscillation channel: {} -> {}",
                 from_type, to_type);
  }

  if (params.count("osc:numubar_to_nuebar")) {
    from_type = GetNuType(-14);
    to_type = GetNuType(-12);
    log_info("[Prob3plusplusWeightCalc]: Set numubar to nuebar appearance "
                 "oscillation channel: {} -> {}",
                 from_type, to_type);
  }

  if (params.count("baseline:t2k")) {
    set_parameters({{"baseline_km", 295}});
    log_info("[Prob3plusplusWeightCalc]: Set t2k baseline");
  }
  if (params.count("baseline:DUNE")) {
    set_parameters({{"baseline_km", 1300}});
    log_info("[Prob3plusplusWeightCalc]: Set DUNE baseline");
  }
  if (params.count("baseline:NOvA")) {
    set_parameters({{"baseline_km", 810}});
    log_info("[Prob3plusplusWeightCalc]: Set NOvA baseline");
  }

  if (params.count("t2k:bestfit")) {
    set_parameters({{"sinsq_th12", 0.297},
                    {"sinsq_th13", 0.0214},
                    {"sinsq_th23", 0.526},
                    {"dmsq_21", 7.37E-5},
                    {"dmsq_atm", 2.463E-3},
                    {"dcp_rad", 0}});
    log_info("[Prob3plusplusWeightCalc]: Set T2K bestfit parameters");
  }

  if (params.count("NuFit:5.2")) {
    set_parameters({{"sinsq_th12", 0.303},
                    {"sinsq_th13", 0.02203},
                    {"sinsq_th23", 0.572},
                    {"dmsq_21", 7.41E-5},
                    {"dmsq_atm", 2.51E-3},
                    {"dcp_rad", 0}});
    log_info("[Prob3plusplusWeightCalc]: Set NuFit:5.2 bestfit parameters");
  }
}

BOOST_DLL_ALIAS(nuis::Prob3plusplusWeightCalc::MakeWeightCalc, MakeWeightCalc);

} // namespace nuis
