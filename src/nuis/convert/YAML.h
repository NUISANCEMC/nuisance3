#pragma once

#include "yaml-cpp/yaml.h"

#include "nuis/histframe/BinnedValues.h"

#include <string>

namespace nuis {
YAML::Node to_yaml(BinnedValues const &hf);
std::string to_yaml_str(BinnedValues const &hf);

BinnedValues from_yaml(YAML::Node const &yhf);
BinnedValues from_yaml_str(std::string const &shf);

Eigen::ArrayXXd covar_from_yaml(YAML::Node const &yhf);
Eigen::ArrayXXd covar_from_yaml_str(std::string const &shf);
} // namespace nuis