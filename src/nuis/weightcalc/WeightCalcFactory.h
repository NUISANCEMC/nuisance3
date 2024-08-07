#pragma once

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/function.hpp"
#endif

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <map>

namespace nuis {

class IEventSource;
using IEventSourcePtr = std::shared_ptr<IEventSource>;
class IEventSourceWrapper;
using IWrappedEventSourcePtr = std::shared_ptr<IEventSourceWrapper>;

class WeightCalcFactory : public nuis_named_log("WeightCalc") {

#ifdef NUISANCE_USE_BOOSTDLL
  using IWeightCalc_PluginFactory_t = IWeightCalcPluginPtr(IEventSourcePtr,
                                                           YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IWeightCalc_PluginFactory_t>>
      pluginfactories;
#endif

public:
  WeightCalcFactory();

  IWeightCalcHM3MapPtr make(IEventSourcePtr evs, YAML::Node const &cfg = {});
  IWeightCalcHM3MapPtr make(IWrappedEventSourcePtr evs,
                            YAML::Node const &cfg = {});
};
} // namespace nuis