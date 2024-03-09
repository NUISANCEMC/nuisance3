#include "ProSelecta/ProSelecta.h"

#include "nuis/record/Utility.h"

#include "nuis/except.h"
#include "nuis/log.txx"

namespace nuis {

NEW_NUISANCE_EXCEPT(PROSELECTA_DIRUndefined);
NEW_NUISANCE_EXCEPT(NUISANCEDBUndefined);
NEW_NUISANCE_EXCEPT(InvalidYAMLData);
NEW_NUISANCE_EXCEPT(InvalidYAMLSchemaObject);

std::string database() {
  // Require ProSelectra input
  auto PROSELECTA = std::getenv("PROSELECTA_DIR");
  (void)PROSELECTA;
  if (!PROSELECTA) {
    log_critical("PROSELECTA_DIR environment variable not defined");
    throw PROSELECTA_DIRUndefined();
  }
  ProSelecta::Get().AddIncludePath(PROSELECTA);

  // Require Database Valid
  auto DATABASE = std::getenv("NUISANCEDB");
  (void)DATABASE;
  if (!DATABASE) {
    log_critical("NUISANCE_DB environment variable not defined");
    throw NUISANCEDBUndefined();
  }

  return std::string(DATABASE);
}

using yamlvlogger = nuis_named_log("YAMLValid");

// This should be made more robust using proper schema
bool validate_yaml_map(std::string source, const YAML::Node &schema,
                       const YAML::Node &data, const std::string &path) {
  if (!schema.IsMap()) {
    yamlvlogger::log_critical("Both schema and data should be YAML maps.");
    throw InvalidYAMLSchemaObject();
  }
  if (!data.IsMap()) {
    yamlvlogger::log_critical("Both schema and data should be YAML maps.");
    throw InvalidYAMLData();
  }
  bool good_yaml = true;

  for (const auto &pair : schema) {
    std::string key = pair.first.as<std::string>();
    YAML::Node schemaValue = pair.second;
    YAML::Node dataValue = data[key];

    std::string fullPath = path.empty() ? key : path + "." + key;

    // Check for missing key in data
    if (!dataValue) {

      if (good_yaml) {
        yamlvlogger::log_error("Error in {} YAML config node.", source);
        good_yaml = false;
      }

      yamlvlogger::log_warn("Missing key '{}' in YAML data.", fullPath);
      continue;
    }

    // Validate types
    if (schemaValue.Type() != dataValue.Type()) {

      if (!good_yaml) {
        yamlvlogger::log_error("Error in {} YAML config node.", source);
        good_yaml = false;
      }

      std::cout << "Error: Type mismatch for key '" << fullPath
                << "'. Expected " << schemaValue.Type() << " but found "
                << dataValue.Type() << "." << std::endl;
      continue;
    }

    // Recursive check for nested maps
    if (schemaValue.IsMap()) {
      validate_yaml_map(source, schemaValue, dataValue, fullPath);
    }
  }
  if (!good_yaml) {
    yamlvlogger::log_critical("Please fix the YAML. Template is : {}",
                              nuis::str_via_ss(schema));
    throw InvalidYAMLData();
  }

  return good_yaml;
}

} // namespace nuis