#pragma once

#include <memory>
#include <string>

#include "yaml-cpp/yaml.h"

#include "ProSelecta/ProSelecta.h"

#include "nuis/record/IAnalysis.h"

#include "nuis/log.h"

namespace nuis {
struct IRecord : public nuis_named_log("Record") {
  IRecord() {}

  IRecord(YAML::Node) {}

  virtual AnalysisPtr analysis(std::string const &name) {
    YAML::Node cfg = node;
    cfg["analysis"] = name;
    return analysis(cfg);
  }

  virtual std::vector<std::string> get_analyses() const = 0;

  virtual AnalysisPtr analysis(YAML::Node const &cfg) = 0;

  AnalysisPtr operator[](std::string const &name) { return analysis(name); }

  YAML::Node node;
};

using IRecordPtr = std::shared_ptr<IRecord>;

} // namespace nuis
