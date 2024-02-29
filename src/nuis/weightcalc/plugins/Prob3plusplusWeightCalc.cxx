#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "NuHepMC/EventUtils.hxx"

#include "HepMC3/GenParticle.h"

#include "BargerPropagator.h"

#include "boost/dll/alias.hpp"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

namespace {
constexpr double deg2rad = asin(1) / 90.0;
constexpr double REarth_km = 6371.393;
} // namespace

enum NuTypes {
  kNuebarType = -1,
  kNumubarType = -2,
  kNutaubarType = -3,
  kNueType = 1,
  kNumuType = 2,
  kNutauType = 3,
  kInvalid = 0
};

NuTypes GetNuType(int pdg) {
  switch (pdg) {
  case 16:
    return kNutauType;
  case 14:
    return kNumuType;
  case 12:
    return kNueType;
  case -16:
    return kNutaubarType;
  case -14:
    return kNumubarType;
  case -12:
    return kNuebarType;
  default: {
    spdlog::warn("[Prob3plusplusWeightCalc]: Invalid neutrino PDG code: {}",
                 pdg);
    return kInvalid;
  }
  }
}

class Prob3plusplusWeightCalc : public IWeightCalcPlugin {
  std::unique_ptr<BargerPropagator> prop;

  double DipAngle_degrees;         // = 5.8;
  std::array<double, 6> OscParams; // = {0.825, 0.10, 1.0, 7.9e-5, 2.5e-3, 0.0};

  double sinsq_th12;
  double sinsq_th13;
  double sinsq_th23;

  double dmsq_21;
  double dmsq_atm;

  double dcp_rad;

  double LengthParam;

  NuTypes FromType, ToType;

  void set_dipangle(double dip_angle_deg) {
    LengthParam = std::cos((90.0 + dip_angle_deg) * deg2rad);
  }
  void set_baseline(double baseline_km) {
    set_dipangle(std::asin(baseline_km / (2.0 * REarth_km)) / deg2rad);
  }

public:
  double calc_weight(HepMC3::GenEvent const &ev) {
    auto beamp = NuHepMC::Event::GetBeamParticle(ev);
    if (!beamp) {
      spdlog::warn("[Prob3plusplusWeightCalc]: Failed to find valid beam "
                   "particle in event");
      return 1;
    }

    FromType = GetNuType(beamp->pid());
    ToType = GetNuType(beamp->pid());

    prop->SetMNS(sinsq_th12, sinsq_th13, sinsq_th23, dmsq_21, dmsq_atm, dcp_rad,
                 beamp->momentum().e() * NuHepMC::Event::ToMeVFactor(ev) * 1E-3,
                 true, FromType);
    prop->DefinePath(LengthParam, 0);
    prop->propagate(ToType);

    return prop->GetProb(FromType, ToType);
  }

  void set_parameters(std::map<std::string, double> const &params) {

    if (params.count("sinsq_th12")) {
      sinsq_th12 = params.at("sinsq_th12");
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th12 = {}",
                   sinsq_th12);
    }
    if (params.count("th12")) {
      sinsq_th12 = std::pow(std::sin(params.at("sinsq_th12")), 2);
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th12 = {}",
                   sinsq_th12);
    }

    if (params.count("sinsq_th13")) {
      sinsq_th13 = params.at("sinsq_th13");
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th13 = {}",
                   sinsq_th13);
    }
    if (params.count("th13")) {
      sinsq_th13 = std::pow(std::sin(params.at("sinsq_th13")), 2);
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th13 = {}",
                   sinsq_th13);
    }

    if (params.count("sinsq_th23")) {
      sinsq_th23 = params.at("sinsq_th23");
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th23 = {}",
                   sinsq_th23);
    }
    if (params.count("th23")) {
      sinsq_th23 = std::pow(std::sin(params.at("sinsq_th23")), 2);
      spdlog::info("[Prob3plusplusWeightCalc]: Set sinsq_th23 = {}",
                   sinsq_th23);
    }

    if (params.count("dmsq_21")) {
      dmsq_21 = params.at("dmsq_21");
      spdlog::info("[Prob3plusplusWeightCalc]: Set dmsq_21 = {}", dmsq_21);
    }
    if (params.count("dmsq_atm")) {
      dmsq_atm = params.at("dmsq_atm");
      spdlog::info("[Prob3plusplusWeightCalc]: Set dmsq_atm = {}", dmsq_atm);
    }

    if (params.count("dcp_rad")) {
      dcp_rad = params.at("dcp_rad");
      spdlog::info("[Prob3plusplusWeightCalc]: Set dcp_rad = {}", dcp_rad);
    }
    if (params.count("dcp_npi")) {
      dcp_rad = params.at("dcp_npi") * M_PI;
      spdlog::info(
          "[Prob3plusplusWeightCalc]: Set dcp_rad = {}, from dcp_npi = {}",
          dcp_rad, params.at("dcp_npi"));
    }

    if (params.count("baseline_km")) {
      set_baseline(params.at("baseline_km"));
      spdlog::info("[Prob3plusplusWeightCalc]: Set baseline_km = {}",
                   params.at("baseline_km"));
    }

    if (params.count("dip_angle_deg")) {
      set_dipangle(params.at("dip_angle_deg"));
      spdlog::info("[Prob3plusplusWeightCalc]: Set dip_angle_deg = {}",
                   params.at("dip_angle_deg"));
    }

    if (params.count("baseline:t2k")) {
      set_parameters({{"baseline_km", 295}});
      spdlog::info("[Prob3plusplusWeightCalc]: Set t2k baseline");
    }
    if (params.count("baseline:DUNE")) {
      set_parameters({{"baseline_km", 1300}});
      spdlog::info("[Prob3plusplusWeightCalc]: Set DUNE baseline");
    }
    if (params.count("baseline:NOvA")) {
      set_parameters({{"baseline_km", 810}});
      spdlog::info("[Prob3plusplusWeightCalc]: Set NOvA baseline");
    }

    if (params.count("bestfit:t2k")) {
      set_parameters({{"sinsq_th12", 0.297},
                      {"sinsq_th13", 0.0214},
                      {"sinsq_th23", 0.526},
                      {"dmsq_21", 7.37E-5},
                      {"dmsq_atm", 2.463E-3},
                      {"dcp_rad", 0}});
      spdlog::info("[Prob3plusplusWeightCalc]: Set T2K bestfit parameters");
    }
  }

  bool good() const { return true; }

  Prob3plusplusWeightCalc(YAML::Node const &) {
    prop = std::make_unique<BargerPropagator>();
  };

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr,
                                             YAML::Node const &cfg) {
    return std::make_shared<Prob3plusplusWeightCalc>(cfg);
  }

  virtual ~Prob3plusplusWeightCalc() {}
};

BOOST_DLL_ALIAS(nuis::Prob3plusplusWeightCalc::MakeWeightCalc, MakeWeightCalc);

} // namespace nuis
