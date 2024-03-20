#pragma once

#include <memory>
#include <string>

#include "yaml-cpp/yaml.h"

#include "ProSelecta/ProSelecta.h"

#include "nuis/record/Table.h"

#include "nuis/log.h"

namespace nuis {

using TablePtr = std::shared_ptr<Table>;

struct IRecord : public nuis_named_log("Record") {

  IRecord(){};

  IRecord(YAML::Node /*n*/) {
    std::cout << "Base constructor being called" << std::endl;
  };

  virtual TablePtr table(std::string const &name) = 0;

  TablePtr operator[](std::string const &name) { return table(name); }

  YAML::Node node;
};
} // namespace nuis