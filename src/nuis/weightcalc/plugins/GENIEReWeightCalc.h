#pragma once

#include "nuis/eventinput/plugins/GHEP3EventSource.h"
#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "yaml-cpp/yaml.h"

namespace genie {
namespace rew {
class GReWeight;
}
} // namespace neut

namespace nuis {

class GENIEReWeightCalc : public IWeightCalcPlugin {
  std::unique_ptr<genie::rew::GReWeight> fGENIE3RW;
  std::shared_ptr<GHEP3EventSource> nevs;

public:
  double calc_weight(HepMC3::GenEvent const &ev);
  void set_parameters(std::map<std::string, double> const &params);
  bool good() const { return bool(nevs); }

  GENIEReWeightCalc(IEventSourcePtr evs, YAML::Node const &);

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr evs,
                                             YAML::Node const &cfg);

  virtual ~GENIEReWeightCalc();
};

} // namespace nuis
