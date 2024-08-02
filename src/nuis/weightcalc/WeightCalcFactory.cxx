#include "nuis/eventinput/IEventSourceWrapper.h"

#include "nuis/weightcalc/WeightCalcFactory.h"

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
  auto NUISANCE = std::getenv("NUISANCE3_ROOT");

  if (!NUISANCE) {
    log_critical("NUISANCE_ROOT environment variable not defined");
    throw NUISANCE_ROOTUndefined();
  }
#ifdef NUISANCE_USE_BOOSTDLL
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
IWeightCalcPluginPtr TryAllKnownPlugins(IEventSourcePtr evs,
                                        YAML::Node const &cfg) {
  IWeightCalcPluginPtr ws;

  if (!cfg["plugin_name"] ||
      cfg["plugin_name"].as<std::string>() == "GENIEReWeight") {
    ws = GENIEReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin GENIEReWeightCalc is able to weight input file");
      return ws;
    }
  }
  if (!cfg["plugin_name"] ||
      cfg["plugin_name"].as<std::string>() == "Prob3plusplus") {
    ws = Prob3plusplusWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin Prob3plusplusWeightCalc is able to weight input file");
      return ws;
    }
  }
  if (!cfg["plugin_name"] ||
      cfg["plugin_name"].as<std::string>() == "NReWeight") {
    ws = NReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin NReWeightCalc is able to weight input file");
      return ws;
    }
  }
  if (!cfg["plugin_name"] ||
      cfg["plugin_name"].as<std::string>() == "T2KReWeight") {
    ws = T2KReWeightCalc::MakeWeightCalc(evs, cfg);
    if (ws->good()) {
      log_debug("Plugin T2KReWeightCalc is able to weight input file");
      return ws;
    }
  }
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
  return TryAllKnownPlugins(evs, cfg);
#endif
}

IWeightCalcHM3MapPtr WeightCalcFactory::make(IWrappedEventSourcePtr evs,
                                             YAML::Node const &cfg) {
  return make(evs->unwrap(), cfg);
}
} // namespace nuis