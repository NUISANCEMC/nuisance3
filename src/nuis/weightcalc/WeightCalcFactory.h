#pragma once

#include "nuis/weightcalc/IWeightCalcContainer.h"
#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "boost/dll/import.hpp"
#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <regex>

namespace nuis {
class WeightCalcFactory {

  using IWeightCalc_PluginFactory_t = IWeightCalcPluginPtr(IEventSourcePtr,
                                                           YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IWeightCalc_PluginFactory_t>>
      pluginfactories;

public:
  WeightCalcFactory() {
    auto NUISANCE = std::getenv("NUISANCE_ROOT");

    if (!NUISANCE) {
      spdlog::critical("NUISANCE_ROOT environment variable not defined");
      abort();
    }

    std::filesystem::path shared_library_dir{NUISANCE};
    shared_library_dir /= "lib/plugins";
    std::regex plugin_re("nuisplugin-weightcalc-.*.so");
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{shared_library_dir}) {
      if (std::regex_match(dir_entry.path().filename().native(), plugin_re)) {
        spdlog::info("Found weightcalc plugin: {}", dir_entry.path().native());
        pluginfactories.emplace(
            dir_entry.path(),
            boost::dll::import_alias<IWeightCalc_PluginFactory_t>(
                dir_entry.path().native(), "MakeWeightCalc"));
      }
    }
  }

  IWeightCalcHM3MapPtr make_all(IEventSourcePtr evs,
                                YAML::Node const &cfg = {}) {
    if (!evs) {
      return nullptr;
    }

    auto matching_plugins = std::make_shared<IWeightCalcContainerHM3Map>();

    for (auto &[pluginso, plugin] : pluginfactories) {
      auto wc = plugin(evs, cfg);
      if (wc->good()) {
        spdlog::info("Found WeightCalculator plugin: {} that can process "
                     " proffered IEventSource",
                     pluginso.native());
        matching_plugins->add(wc);
      }
    }

    return matching_plugins;
  }

  IWeightCalcHM3MapPtr make_all(IWrappedEventSourcePtr evs,
                                YAML::Node const &cfg = {}) {
    return make_all(evs->unwrap(), cfg);
  }

  IWeightCalcHM3MapPtr make(IEventSourcePtr evs, YAML::Node const &cfg = {}) {
    if (!evs) {
      return nullptr;
    }

    if (cfg["plugin_name"]) {
      std::string plugin_name = cfg["plugin_name"].as<std::string>();
      for (auto &[pluginso, plugin] : pluginfactories) {
        if (pluginso.filename().native() ==
            (std::string("nuisplugin-weightcalc-") + plugin_name + ".so")) {
          auto wc = plugin(evs, cfg);
          if (wc->good()) {
            return wc;
          } else {
            spdlog::critical(
                "Explicitly asked for weightcalc plugin: {}, but it failed to "
                "configure itself with the proferred event source.");
            abort();
          }
        }
      }
      spdlog::critical(
          "Explicitly asked for weightcalc plugin: {}, but do not have {}",
          plugin_name,
          (std::string("nuisplugin-weightcalc-.") + plugin_name + ".so"));
      abort();
    }

    for (auto &[pluginso, plugin] : pluginfactories) {
      auto wc = plugin(evs, cfg);
      if (wc->good()) {
        spdlog::info("Found WeightCalculator plugin: {} that can process "
                     " proffered IEventSource",
                     pluginso.native());
        return wc;
      }
    }

    return nullptr;
  }

  IWeightCalcHM3MapPtr make(IWrappedEventSourcePtr evs,
                            YAML::Node const &cfg = {}) {
    return make(evs->unwrap(), cfg);
  }
};
} // namespace nuis