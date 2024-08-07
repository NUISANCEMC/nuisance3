#pragma once

#include "nuis/record/plugins/IRecordPlugin.h"

#include "yaml-cpp/yaml.h"

namespace nuis {

class HEPDataRecord : public IRecordPlugin {
public:
  HEPDataRecord(YAML::Node const &cfg);

  TablePtr table(YAML::Node const &cfg_in);

  bool good() const { return true; }

  static IRecordPluginPtr MakeRecord(YAML::Node const &cfg);

  virtual ~HEPDataRecord();
};

} // namespace nuis
