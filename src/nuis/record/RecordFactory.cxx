#include "nuis/record/RecordFactory.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/import.hpp"
#include "boost/dll/runtime_symbol_info.hpp"
#else
#include "nuis/record/plugins/plugins.h"
#endif

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "nuis/env.h"
#include "nuis/log.txx"

#include <regex>
#include <string>

namespace nuis {

RecordFactory::RecordFactory() {
#ifdef NUISANCE_USE_BOOSTDLL
  auto NUISANCE = env::NUISANCE3_ROOT();

  std::filesystem::path shared_library_dir{NUISANCE};
  shared_library_dir /= "lib/plugins";
  std::regex plugin_re("nuisplugin-record-.*.so");
  for (auto const &dir_entry :
       std::filesystem::directory_iterator{shared_library_dir}) {
    if (std::regex_match(dir_entry.path().filename().native(), plugin_re)) {
      log_debug("Found record plugin: {}", dir_entry.path().native());
      pluginfactories.emplace(dir_entry.path(),
                              boost::dll::import_alias<IRecord_PluginFactory_t>(
                                  dir_entry.path().native(), "Make"));
    }
  }
#endif
}

#ifndef NUISANCE_USE_BOOSTDLL
IRecordPtr TryAllKnownRecordPlugins(YAML::Node const &cfg) {

  bool type_specified = bool(cfg["type"]);
  std::string const &type_name =
      type_specified ? cfg["type"].as<std::string>() : "";

#ifdef NUIS_RECORD_HEPData_Enabled
  if (type_name == "hepdata") {
    auto rec = HEPDataRecordPlugin::MakeRecord(cfg);
    if (!rec->good()) {
      log_error("record plugin {} requested cannot be instantiated with "
                "proferred config.",
                type_name);
    }
    return rec;
  }
#endif

#ifdef NUIS_RECORD_NUISANCE2_Enabled
  if (type_name == "nuisance2") {
    auto rec = NUISANCE2Record::MakeRecord(cfg);
    if (!rec->good()) {
      log_error("record plugin {} requested cannot be instantiated with "
                "proferred config.",
                type_name);
    }
    return rec;
  }
#endif

  return nullptr;
}
#endif

IRecordPtr RecordFactory::make(YAML::Node cfg) {

  if (!cfg["type"]) {
    throw std::runtime_error("RecordFactory::make called without a type field "
                             "in the configuration document.");
  }

#ifdef NUISANCE_USE_BOOSTDLL
  std::string record_type =
      "nuisplugin-record-" + cfg["type"].as<std::string>() + ".so";

  for (auto &[pluginso, plugin] : pluginfactories) {
    std::string fullpath = std::string(pluginso);
    if (fullpath.find(record_type) != std::string::npos) {
      auto rec = plugin(cfg);
      if (!rec->good()) {
        log_error("record plugin {} requested cannot be instantiated with "
                  "proferred config.",
                  cfg["type"].as<std::string>());
      }
      return rec;
    }
  }
  return nullptr;
#else
  return TryAllKnownRecordPlugins(cfg);
#endif
}

AnalysisPtr RecordFactory::make_analysis(YAML::Node cfg) {
  auto record = make(cfg);
  return record->analysis(cfg);
}

} // namespace nuis
