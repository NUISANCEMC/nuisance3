#pragma once

#include "spdlog/spdlog.h"


namespace nuis {

std::string database() {
  // Require ProSelectra input
  auto PROSELECTA = std::getenv("PROSELECTA_DIR");
  if (!PROSELECTA) {
    spdlog::critical("PROSELECTA_DIR environment variable not defined");
    abort();
  } 
  ProSelecta::Get().AddIncludePath(PROSELECTA);


  // Require Database Valid
  auto DATABASE = std::getenv("NUISANCEDB");
  if (!DATABASE) {
    spdlog::critical("NUISANCE_DB environment variable not defined");
    abort();
  }

  return std::string(DATABASE);
}

// This should be made more robust using proper schema
bool validate_yaml_map(std::string source, const YAML::Node& schema, const YAML::Node& data, const std::string& path = "") {
    if (!schema.IsMap() || !data.IsMap()) {
        std::cout << "Error: Both schema and data should be YAML maps." << std::endl;
        abort();
        return false;
    }
    bool good_yaml = true;

    for (const auto& pair : schema) {
        std::string key = pair.first.as<std::string>();
        YAML::Node schemaValue = pair.second;
        YAML::Node dataValue = data[key];

        std::string fullPath = path.empty() ? key : path + "." + key;

        // Check for missing key in data
        if (!dataValue) {

            if (good_yaml){
                std::cout << "Error in " << source << " YAML config node." << std::endl;
                good_yaml = false;
            }
            
            std::cout << "Error: Missing key '" << fullPath << "' in data." << std::endl;
            continue;
        }

        // Validate types
        if (schemaValue.Type() != dataValue.Type()) {

            if (!good_yaml){
                std::cout << "Error in " << source << " YAML config node." << std::endl;
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
    if (!good_yaml){

            std::cout << "Aborting. Please fix the YAML. Template is : " << std::endl;
            std::cout << schema << std::endl;
            abort();
        }
    
    return good_yaml;
}


}