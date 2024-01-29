#include "nuis/eventinput/HepMC3EventSource.h"

#include "boost/dll/import.hpp"
#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <regex>

namespace nuis {
class EventSourceFactory {

  typedef IEventSourcePtr(IEventSource_PluginFactory_t)(YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IEventSource_PluginFactory_t>>
      pluginfactories;

public:
  EventSourceFactory() {
    auto NUIS_PLUGINS = std::getenv("NUIS_PLUGINS");

    if (!NUIS_PLUGINS) {
      spdlog::critical("NUIS_PLUGINS environment variable not defined");
      abort();
    }

    const std::filesystem::path shared_library_dir{NUIS_PLUGINS};
    std::regex plugin_re("nuisplugin.eventinput.*.so");
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{shared_library_dir}) {
      if (std::regex_match(dir_entry.path().filename().native(), plugin_re)) {
        spdlog::info("Found eventinput plugin: {}", dir_entry.path().native());
        pluginfactories.emplace(
            dir_entry.path(),
            boost::dll::import_alias<IEventSource_PluginFactory_t>(
                dir_entry.path().native(), "MakeEventSource"));
      }
    }
  }

  IEventSourcePtr Make(YAML::Node const &cfg) {
    for (auto &[pluginso, plugin] : pluginfactories) {
      auto es = plugin(cfg);
      if (es->first()) {
        spdlog::info("Reading file {} with plugin {}",
                     cfg["filepath"].as<std::string>(), pluginso.native());
        return es;
      }
    }
    // try plugins first as there is a bug in HepMC3 root reader that segfaults
    // if it is not passed the expected type.
    auto es =
        std::make_shared<HepMC3EventSource>(cfg["filepath"].as<std::string>());
    if (es->first()) {
      spdlog::info("Reading file {} with native HepMC3EventSource",
                   cfg["filepath"].as<std::string>());
      return es;
    }
    return nullptr;
  }

  IEventSourcePtr Make(std::string const &filepath) {
    return Make(YAML::Load(fmt::format(R"(
    filepath: {}
    )",
                                       filepath)));
  }
};
} // namespace nuis