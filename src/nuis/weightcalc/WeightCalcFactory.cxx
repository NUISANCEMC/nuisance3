#include "nuis/eventinput/IEventSourceWrapper.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

#include "nuis/env.h"
#include "nuis/except.h"
#include "nuis/log.txx"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/import.hpp"
#else
#include "nuis/weightcalc/plugins/plugins.h"
#endif

#include <regex>

NEW_NUISANCE_EXCEPT(NUISANCE_ROOTUndefined);
NEW_NUISANCE_EXCEPT(UnableToProvisionWeightCalcPlugin);
NEW_NUISANCE_EXCEPT(InvalidWeightCalcPluginRequested);

namespace nuis {
WeightCalcFactory::WeightCalcFactory() {

#ifdef NUISANCE_USE_BOOSTDLL
  auto NUISANCE = env::NUISANCE3_ROOT();

  std::filesystem::path shared_library_dir{NUISANCE};
  shared_library_dir /= "lib/plugins";
  std::regex plugin_re("nuisplugin-weightcalc-.*.so");
  for (auto const &dir_entry :
       std::filesystem::directory_iterator{shared_library_dir}) {
    if (std::regex_match(dir_entry.path().filename().native(), plugin_re)) {
      log_info("Found weightcalc plugin: {}", dir_entry.path().native());
      pluginfactories.emplace(
          dir_entry.path(),
          boost::dll::import_alias<IWeightCalc_PluginFactory_t>(
              dir_entry.path().native(), "MakeWeightCalc"));
    }
  }
#endif
}

#ifndef NUISANCE_USE_BOOSTDLL
IWeightCalcPluginPtr TryAllKnownWeightPlugins(IEventSourcePtr evs,
                                              YAML::Node const &cfg) {
  bool plugin_specified = bool(cfg["plugin_name"]);
  std::string const &plugin_name =
      plugin_specified ? cfg["plugin_name"].as<std::string>() : "";

#ifdef NUIS_WEIGHT_CALC_GENIEReWeight_Enabled
  if (!plugin_specified || plugin_name == "GENIEReWeight") {
    auto ws = GENIEReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin GENIEReWeightCalc is able to weight input file");
      return ws;
    }
  }
#endif

#ifdef NUIS_WEIGHT_CALC_Prob3plusplus_Enabled
  if (!plugin_specified || plugin_name == "Prob3plusplus") {
    auto ws = Prob3plusplusWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin Prob3plusplusWeightCalc is able to weight input file");
      return ws;
    }
  }
#endif

#ifdef NUIS_WEIGHT_CALC_NReWeight_Enabled

  if (!plugin_specified || plugin_name == "NReWeight") {
    auto ws = NReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin NReWeightCalc is able to weight input file");
      return ws;
    }
  }
#endif

#ifdef NUIS_WEIGHT_CALC_T2KReWeight_Enabled
  if (!plugin_specified || plugin_name == "T2KReWeight") {
    auto ws = T2KReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin T2KReWeightCalc is able to weight input file");
      return ws;
    }
  }
#endif
  return nullptr;
}
#endif

IWeightCalcHM3MapPtr WeightCalcFactory::make(IEventSourcePtr evs,
                                             YAML::Node const &cfg) {
  if (!evs) {
    return nullptr;
  }

#ifdef NUISANCE_USE_BOOSTDLL
  if (cfg["plugin_name"]) {
    std::string plugin_name = cfg["plugin_name"].as<std::string>();
    for (auto &[pluginso, plugin] : pluginfactories) {
      if (pluginso.filename().native() ==
          (std::string("nuisplugin-weightcalc-") + plugin_name + ".so")) {
        auto wc = plugin(evs, cfg);
        if (wc->good()) {
          return wc;
        } else {
          log_critical(
              "Explicitly asked for weightcalc plugin: {}, but it failed to "
              "configure itself with the proferred event source.");
          throw UnableToProvisionWeightCalcPlugin();
        }
      }
    }
    log_critical(
        "Explicitly asked for weightcalc plugin: {}, but do not have {}",
        plugin_name,
        (std::string("nuisplugin-weightcalc-.") + plugin_name + ".so"));
    throw InvalidWeightCalcPluginRequested();
  }

  for (auto &[pluginso, plugin] : pluginfactories) {
    auto wc = plugin(evs, cfg);
    if (wc->good()) {
      log_info("Found WeightCalculator plugin: {} that can process "
               " proffered IEventSource",
               pluginso.native());
      return wc;
    }
  }

  return nullptr;
#else
  return TryAllKnownWeightPlugins(evs, cfg);
#endif
}

IWeightCalcHM3MapPtr WeightCalcFactory::make(IWrappedEventSourcePtr evs,
                                             YAML::Node const &cfg) {
  return make(evs->unwrap(), cfg);
}
} // namespace nuis