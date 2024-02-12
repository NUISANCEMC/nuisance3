#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/eventinput/HepMC3EventSource.h"

#include "boost/dll/import.hpp"
#include "boost/dll/runtime_symbol_info.hpp"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <regex>

namespace nuis {

EventSourceFactory::EventSourceFactory() {
  auto NUISANCE = std::getenv("NUISANCE_ROOT");

  if (!NUISANCE) {
    spdlog::critical("NUISANCE_ROOT environment variable not defined");
    abort();
  }

  std::filesystem::path shared_library_dir{NUISANCE};
  shared_library_dir /= "lib/plugins";
  std::regex plugin_re("nuisplugin-eventinput-.*.so");
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

std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
EventSourceFactory::MakeUnNormalized(YAML::Node const &cfg) {
  for (auto &[pluginso, plugin] : pluginfactories) {
    auto es = plugin(cfg);
    if (es->first()) {
      spdlog::info("Reading file {} with plugin {}",
                   cfg["filepath"].as<std::string>(), pluginso.native());
      return {es->first().value().run_info(), es};
    }
  }
  // try plugins first as there is a bug in HepMC3 root reader that segfaults
  // if it is not passed the expected type.
  auto es =
      std::make_shared<HepMC3EventSource>(cfg["filepath"].as<std::string>());
  if (es->first()) {
    spdlog::info("Reading file {} with native HepMC3EventSource",
                 cfg["filepath"].as<std::string>());
    return {es->first().value().run_info(), es};
  }
  spdlog::warn("Failed to find plugin capable of reading input file: {}.",
               cfg["filepath"].as<std::string>());
  return {nullptr, nullptr};
}

std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
EventSourceFactory::MakeUnNormalized(std::string const &filepath) {
  return MakeUnNormalized(YAML::Load(fmt::format(R"(
    filepath: {}
    )",
                                                 filepath)));
}
std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
EventSourceFactory::Make(YAML::Node const &cfg) {
  auto [gri, es] = MakeUnNormalized(cfg);
  auto nes = std::make_shared<INormalizedEventSource>(es);
  if (nes->first()) {
    return {gri, nes};
  }
  return {nullptr, nullptr};
}
std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
EventSourceFactory::Make(std::string const &filepath) {
  return Make(YAML::Load(fmt::format(R"(
    filepath: {}
    )",
                                     filepath)));
}
} // namespace nuis