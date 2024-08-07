#pragma once

#include "nuis/record/plugins/IRecordPlugin.h"

#include "yaml-cpp/yaml.h"

namespace nuis {

class NUISANCE2Record : public IRecordPlugin {
public:

  explicit NUISANCE2Record(YAML::Node const &cfg);

  TablePtr table(YAML::Node const &cfg);

  bool good() const { return true; }

  static IRecordPluginPtr MakeRecord(YAML::Node const &cfg);

  virtual ~NUISANCE2Record();
};

} // namespace nuis
