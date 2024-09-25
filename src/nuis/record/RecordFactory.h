#pragma once

#include "nuis/record/IRecord.h"
#include "nuis/record/IAnalysis.h"

#include "nuis/log.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/function.hpp"
#endif

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace nuis {

class RecordFactory : public nuis_named_log("Record") {

#ifdef NUISANCE_USE_BOOSTDLL
  using IRecord_PluginFactory_t = IRecordPtr(YAML::Node const &);
  std::map<std::filesystem::path, boost::function<IRecord_PluginFactory_t>>
      pluginfactories;
#endif

public:
  RecordFactory();
  IRecordPtr make(YAML::Node cfg);
  AnalysisPtr make_analysis(YAML::Node cfg);
};
} // namespace nuis
