#pragma once

#include "yaml-cpp/yaml.h"

#include <string>

namespace nuis {

// This should be made more robust using proper schema
bool validate_yaml_map(std::string source, const YAML::Node &schema,
                       const YAML::Node &data, const std::string &path = "");

} // namespace nuis