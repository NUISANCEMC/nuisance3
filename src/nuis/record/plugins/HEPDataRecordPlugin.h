#pragma once

#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/HEPData/Record.hxx"

#include "yaml-cpp/yaml.h"

#include <memory>

namespace nuis {

class HEPDataRecordPlugin : public IRecordPlugin {
public:
  HEPDataRecordPlugin(YAML::Node const &cfg);

  AnalysisPtr analysis(YAML::Node const &cfg_in);

  bool good() const { return true; }

  static IRecordPluginPtr MakeRecord(YAML::Node const &cfg);

  virtual ~HEPDataRecordPlugin();

  HEPData::Record record;
};

} // namespace nuis
