#include "yaml-cpp/yaml.h"

YAML::Node to_yaml(HistFrame const &hf);
std::string to_yaml_str(HistFrame const &hf);

HistFrame from_yaml(YAML::Node const &yhf);
HistFrame from_yaml_str(std::string const &shf);