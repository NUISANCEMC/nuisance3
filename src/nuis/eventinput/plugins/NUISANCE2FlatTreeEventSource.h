#pragma once

#include "nuis/eventinput/IEventSource.h"
#include "yaml-cpp/yaml.h"
namespace nuis {

IEventSourcePtr
NUISANCE2FlatTreeEventSource_MakeEventSource(YAML::Node const &cfg);

} // namespace nuis