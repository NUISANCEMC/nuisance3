#pragma once

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <array>

class BargerPropagator;

namespace nuis {

class Prob3plusplusWeightCalc : public IWeightCalcPlugin {

public:
  enum NuTypes {
    kNuebarType = -1,
    kNumubarType = -2,
    kNutaubarType = -3,
    kNueType = 1,
    kNumuType = 2,
    kNutauType = 3,
    kInvalid = 0
  };

private:
  std::unique_ptr<BargerPropagator> prop;

  double DipAngle_degrees;
  std::array<double, 6> OscParams;

  double sinsq_th12;
  double sinsq_th13;
  double sinsq_th23;

  double dmsq_21;
  double dmsq_atm;

  double dcp_rad;

  double LengthParam;

  NuTypes from_type, to_type;

  void set_dipangle(double dip_angle_deg);
  void set_baseline(double baseline_km);

public:
  double calc_weight(HepMC3::GenEvent const &ev);
  double prob(double enu_GeV);

  void set_parameters(std::map<std::string, double> const &params);

  bool good() const { return true; }

  Prob3plusplusWeightCalc(YAML::Node const &);

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr,
                                             YAML::Node const &cfg) {
    return std::make_shared<Prob3plusplusWeightCalc>(cfg);
  }

  virtual ~Prob3plusplusWeightCalc();
};

} // namespace nuis
