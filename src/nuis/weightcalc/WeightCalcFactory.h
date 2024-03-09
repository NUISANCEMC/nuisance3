#pragma once

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <map>

namespace nuis {
class WeightCalcFactory : public nuis_named_log("WeightCalc") {

  using IWeightCalc_PluginFactory_t = IWeightCalcPluginPtr(IEventSourcePtr,
                                                           YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IWeightCalc_PluginFactory_t>>
      pluginfactories;

public:
  WeightCalcFactory();

  IWeightCalcHM3MapPtr make(IEventSourcePtr evs, YAML::Node const &cfg = {});
  IWeightCalcHM3MapPtr make(IWrappedEventSourcePtr evs,
                            YAML::Node const &cfg = {});
};
} // namespace nuis