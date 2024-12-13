#pragma once

#include "nuis/record/plugins/IRecordPlugin.h"

#include "yaml-cpp/yaml.h"

namespace nuis {

class NUISANCE2Record : public IRecordPlugin {

  std::map<std::string, AnalysisPtr> analyses;
  
public:

  explicit NUISANCE2Record(YAML::Node const &cfg);

  AnalysisPtr analysis(YAML::Node const &cfg);
  std::vector<std::string> get_analyses() const { return {}; }

  bool good() const { return true; }

  static IRecordPluginPtr MakeRecord(YAML::Node const &cfg);

  virtual ~NUISANCE2Record();
};

} // namespace nuis
