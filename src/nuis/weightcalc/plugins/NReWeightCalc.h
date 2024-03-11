#pragma once

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"
#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "CommonBlockIFace.h"
#include "NReWeightFactory.h"

#include "boost/dll/alias.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

class NReWeightCalc : public IWeightCalcPlugin {
public:

  std::unique_ptr<neut::rew::NReWeight> fNEUTRW;
  std::shared_ptr<neutvectEventSource> nevs;

  double calc_weight(HepMC3::GenEvent const &ev);
  void set_parameters(std::map<std::string, double> const &params);
  bool good() const;

  NReWeightCalc(IEventSourcePtr evs, YAML::Node const &cfg);
  

  virtual ~NReWeightCalc();
};

} // namespace nuis
