#pragma once

#include "nuis/eventinput/plugins/GHEP3EventSource.h"
#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "nusystematics/utility/response_helper.hh"

#include "TSpline.h"

#include "yaml-cpp/yaml.h"

namespace nusyst {
class response_helper;
}

namespace nuis {

class nusystematicsWeightCalc : public IWeightCalcPlugin {
public:
  std::unique_ptr<nusyst::response_helper> nusyst;
  std::map<int, std::map<systtools::paramId_t, TSpline3>> splines;
  systtools::param_value_list_t set_params;

  std::shared_ptr<GHEP3EventSource> nevs;

  double calc_weight(HepMC3::GenEvent const &ev);
  void set_parameters(std::map<std::string, double> const &params);
  bool good() const;

  nusystematicsWeightCalc(IEventSourcePtr evs, YAML::Node const &cfg);

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr evs,
                                             YAML::Node const &cfg);

  virtual ~nusystematicsWeightCalc();
};

} // namespace nuis
