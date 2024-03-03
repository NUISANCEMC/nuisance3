#pragma once

#include "boost/function.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <vector>
#include <map>
#include <utility>
#include <memory>

#include "nuis/record/IRecord.h"

namespace nuis {
using IRecordPtr = std::shared_ptr<IRecord>;

class RecordFactory {

  using IRecord_PluginFactory_t = IRecordPtr(YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IRecord_PluginFactory_t>>
      pluginfactories;

 public:

  RecordFactory();
  IRecordPtr make(YAML::Node cfg);
};
} // namespace nuis
