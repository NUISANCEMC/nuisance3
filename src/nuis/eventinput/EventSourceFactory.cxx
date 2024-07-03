#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/eventinput/HepMC3EventSource.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#ifdef USE_BOOSTDLL
#include "boost/dll/import.hpp"
#include "boost/dll/runtime_symbol_info.hpp"
#else
#include "nuis/eventinput/plugins/GHEP3EventSource.h"
#include "nuis/eventinput/plugins/NUISANCE2FlatTreeEventSource.h"
#include "nuis/eventinput/plugins/NuWroevent1EventSource.h"
#include "nuis/eventinput/plugins/neutvectEventSource.h"
#endif

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

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

      log_debug("[EventSourceFactory:PathResolver] -- adding search path: {}",
                path.native());
      nuisance_event_paths.emplace_back(std::move(path));
    }
  }
}

std::filesystem::path PathResolver::resolve(std::string const &filepath) {
  log_debug(
      "[EventSourceFactory:PathResolver]::resolve filepath: {}, exists: {}",
      filepath, std::filesystem::exists(filepath));

  if (!filepath.size()) {
    return {};
  }

  if (std::filesystem::exists(filepath)) {
    return filepath;
  }

  if (filepath.find_first_of(':') <
      filepath.find_first_of('/')) { // assume non-local file
    return filepath;
  }

  if (filepath.front() == '/') {
    std::filesystem::path abspath = filepath;
    log_debug("[EventSourceFactory:PathResolver] abspath: {}, exists: {}",
              abspath.native(), std::filesystem::exists(abspath));
    if (!std::filesystem::exists(abspath)) {
      return {};
    }
    return abspath;
  }

  for (auto const &search_path : nuisance_event_paths) {
    auto path = search_path / filepath;
    log_debug("[EventSourceFactory:PathResolver] search_path: {}, path: "
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
  auto NUISANCE = std::getenv("NUISANCE3_ROOT");

  if (!NUISANCE) {
    log_critical("NUISANCE_ROOT environment variable not defined");
    throw NUISANCE_ROOTUndefined();
  }

#ifdef USE_BOOSTDLL
  std::filesystem::path shared_library_dir{NUISANCE};
  shared_library_dir /= "lib/plugins";
  std::regex plugin_re("nuisplugin-eventinput-.*.so");
  for (auto const &dir_entry :
       std::filesystem::directory_iterator{shared_library_dir}) {
    if (std::regex_match(dir_entry.path().filename().native(), plugin_re)) {
      log_debug("Found eventinput plugin: {}", dir_entry.path().native());
      pluginfactories.emplace(
          dir_entry.path(),
          boost::dll::import_alias<IEventSource_PluginFactory_t>(
              dir_entry.path().native(), "MakeEventSource"));
    }
  }
#endif
}

#ifndef USE_BOOSTDLL
IEventSourcePtr TryAllKnownPlugins(YAML::Node const &cfg) {
  IEventSourcePtr es;

  es = neutvectEventSource_MakeEventSource(cfg);
  if (es->first()) {
    log_debug("Plugin neutvectEventSource is able to read file");
    return es;
  }

  es = GHEP3EventSource_MakeEventSource(cfg);
  if (es->first()) {
    log_debug("Plugin GHEP3EventSource is able to read file");
    return es;
  }

  es = NUISANCE2FlatTreeEventSource_MakeEventSource(cfg);
  if (es->first()) {
    log_debug("Plugin NUISANCE2FlatTreeEventSource is able to read file");
    return es;
  }

  es = NuWroevent1EventSource_MakeEventSource(cfg);
  if (es->first()) {
    log_debug("Plugin NuWroevent1EventSource is able to read file");
    return es;
  }

  return nullptr;
}
#endif

void EventSourceFactory::add_event_path(std::filesystem::path path) {
  if (std::filesystem::exists(path) &&
      (std::find(resolv.nuisance_event_paths.begin(),
                 resolv.nuisance_event_paths.end(),
                 path) == resolv.nuisance_event_paths.end())) {
    log_debug("[EventSourceFactory:PathResolver] -- adding search path: {}",
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
      log_warn("EventSourceFactory::PathResolver did not resolve {} to an "
               "existing filesystem path.",
               cfg["filepath"].as<std::string>());
    }
  } else if (cfg["filepaths"]) {
    std::vector<std::string> filepaths =
        cfg["filepaths"].as<std::vector<std::string>>();
    for (auto &f : filepaths) {
      auto path = resolv.resolve(f);
      if (!path.empty()) {
        f = path.native();
      } else {
        log_warn("EventSourceFactory::PathResolver did not resolve {} to an "
                 "existing filesystem path.",
                 f);
      }
    }
  } else {
    log_warn("[make_unnormalized] was passed no paths.");
    return {nullptr, nullptr};
  }

#ifdef USE_BOOSTDLL
  for (auto &[pluginso, plugin] : pluginfactories) {
    log_trace("Trying plugin {} for file {}", pluginso.native(),
              bool(cfg["filepath"])
                  ? fmt::format("{}", cfg["filepath"].as<std::string>())
                  : fmt::format(
                        "{}", cfg["filepaths"].as<std::vector<std::string>>()));
    auto es = plugin(cfg);
    if (es->first()) {
      log_debug("Plugin {} is able to read file", pluginso.native());
      return {es->first()->run_info(), es};
    }
  }
#else
  auto esp = TryAllKnownPlugins(cfg);
  if (esp) {
    return {esp->first()->run_info(), esp};
  }
#endif

  if (!cfg["filepath"]) {
    log_warn("[make_unnormalized] was only passed a filepaths attribute, "
             "but no plugin was able to read the files. N.B. When reading "
             "HepMC3 files, they must be passed individually via the "
             "filepath attribute as HepMC3 has no equivalent of a TChain.");
    return {nullptr, nullptr};
  }

  // try plugins first as there is a bug in HepMC3 root reader that segfaults
  // if it is not passed the expected type.
  auto es =
      std::make_shared<HepMC3EventSource>(cfg["filepath"].as<std::string>());
  if (es->first()) {
    log_debug("Reading file {} with native HepMC3EventSource",
              cfg["filepath"].as<std::string>());
    return {es->first()->run_info(), es};
  }
  log_warn("Failed to find plugin capable of reading input file: {}.",
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