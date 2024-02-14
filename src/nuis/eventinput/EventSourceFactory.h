#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <vector>

namespace HepMC3 {
class GenRunInfo;
}

namespace nuis {

struct PathResolver {
  std::vector<std::filesystem::path> nuisance_event_paths;
  PathResolver();
  std::filesystem::path resolve(std::string const &filepath);
};

class EventSourceFactory {
  PathResolver resolv;

  using IEventSource_PluginFactory_t = IEventSourcePtr(YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IEventSource_PluginFactory_t>>
      pluginfactories;

public:
  EventSourceFactory();

  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
  Make(YAML::Node const &cfg);
  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, INormalizedEventSourcePtr>
  Make(std::string const &filepath);

  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
  MakeUnNormalized(YAML::Node cfg);
  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
  MakeUnNormalized(std::string const &filepath);
};
} // namespace nuis