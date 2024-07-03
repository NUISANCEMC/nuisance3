#pragma once

#include "nuis/eventinput/IEventSource.h"
#include "yaml-cpp/yaml.h"
namespace nuis {
  
IEventSourcePtr NuWroevent1EventSource_MakeEventSource(YAML::Node const &cfg);

} // namespace nuis