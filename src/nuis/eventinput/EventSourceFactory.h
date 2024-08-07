#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/function.hpp"
#endif

#include "nuis/log.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <vector>

namespace HepMC3 {
class GenRunInfo;
}

namespace nuis {

struct PathResolver : public nuis_named_log("EventInput") {
  std::vector<std::filesystem::path> nuisance_event_paths;
  PathResolver();
  std::filesystem::path resolve(std::string const &filepath);
};

class EventSourceFactory : public nuis_named_log("EventInput") {
  PathResolver resolv;

#ifdef NUISANCE_USE_BOOSTDLL
  using IEventSource_PluginFactory_t = IEventSourcePtr(YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IEventSource_PluginFactory_t>>
      pluginfactories;
#endif

public:
  EventSourceFactory();

  // Add a path to the search path list for input files
  void add_event_path(std::filesystem::path path);

  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
  make(YAML::Node const &cfg);
  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
  make(std::string const &filepath);

  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
  make_unnormalized(YAML::Node cfg);
  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
  make_unnormalized(std::string const &filepath);
};
} // namespace nuis