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

  IWeightCalcHM3MapPtr Make(IEventSourcePtr evs, YAML::Node const &cfg = {}) {
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
};
} // namespace nuis