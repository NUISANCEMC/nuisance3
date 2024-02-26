#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/eventinput/HepMC3EventSource.h"

#include "boost/dll/import.hpp"
#include "boost/dll/runtime_symbol_info.hpp"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <regex>

namespace nuis {

PathResolver::PathResolver() {
  if (std::getenv("NUISANCE_EVENT_PATH")) {
    std::string paths = std::getenv("NUISANCE_EVENT_PATH");
    const std::regex ws_re(":");
    for (auto it =
             std::sregex_token_iterator(paths.begin(), paths.end(), ws_re, -1);
         it != std::sregex_token_iterator(); ++it) {
      std::filesystem::path path = std::string(*it);
      if (path.empty() || !std::filesystem::exists(path)) {
        continue;
      }

      spdlog::info("EventSourceFactory: PathResolver -- adding search path: {}",
                   path.native());
      nuisance_event_paths.emplace_back(std::move(path));
    }
  }
}

std::filesystem::path PathResolver::resolve(std::string const &filepath) {
  spdlog::info(
      "EventSourceFactory: PathResolver::resolve filepath: {}, exists: {}",
      filepath, std::filesystem::exists(filepath));

  if (!filepath.size()) {
    return {};
  }

  if (std::filesystem::exists(filepath)) {
    return filepath;
  }

  if (filepath.front() == '/') {
    std::filesystem::path abspath = filepath;
    spdlog::info("EventSourceFactory: PathResolver abspath: {}, exists: {}",
                 abspath.native(), std::filesystem::exists(abspath));
    if (!std::filesystem::exists(abspath)) {
      return {};
    }
    return abspath;
  }

  for (auto const &search_path : nuisance_event_paths) {
    auto path = search_path / filepath;
    spdlog::info("EventSourceFactory: PathResolver search_path: {}, path: "
                 "{}, exists: {}",
                 search_path.native(), path.native(),
                 std::filesystem::exists(path));
    if (std::filesystem::exists(path)) {
      return path;
    }
  }

  return {};
}

EventSourceFactory::EventSourceFactory() : resolv() {
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

void EventSourceFactory::add_event_path(std::filesystem::path path) {
  if (std::filesystem::exists(path) &&
      (std::find(resolv.nuisance_event_paths.begin(),
                 resolv.nuisance_event_paths.end(),
                 path) == resolv.nuisance_event_paths.end())) {
    spdlog::info("EventSourceFactory: PathResolver -- adding search path: {}",
                 path.native());
    resolv.nuisance_event_paths.emplace_back(std::move(path));
  }
}

std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
EventSourceFactory::make_unnormalized(YAML::Node cfg) {

  if (cfg["filepath"]) {
    auto path = resolv.resolve(cfg["filepath"].as<std::string>());
    if (!path.empty()) {
      cfg["filepath"] = path.native();
    } else {
      spdlog::warn("EventSourceFactory::PathResolver did not resolve {} to an "
                   "existing filesystem path.",
                   cfg["filepath"].as<std::string>());
    }
  }

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
EventSourceFactory::make_unnormalized(std::string const &filepath) {
  return make_unnormalized(YAML::Load(fmt::format(R"(
    filepath: {}
    )",
                                                  filepath)));
}
std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
EventSourceFactory::make(YAML::Node const &cfg) {
  auto [gri, es] = make_unnormalized(cfg);
  auto nes = std::make_shared<INormalizedEventSource>(es);
  if (nes->first()) {
    return {gri, nes};
  }
  return {nullptr, nullptr};
}
std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
EventSourceFactory::make(std::string const &filepath) {
  return make(YAML::Load(fmt::format(R"(
    filepath: {}
    )",
                                     filepath)));
}
} // namespace nuis