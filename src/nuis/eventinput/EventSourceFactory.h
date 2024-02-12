#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace HepMC3 {
class GenRunInfo;
}

namespace nuis {
class EventSourceFactory {

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
  MakeUnNormalized(YAML::Node const &cfg);
  std::pair<std::shared_ptr<HepMC3::GenRunInfo>, IEventSourcePtr>
  MakeUnNormalized(std::string const &filepath);
};
} // namespace nuis