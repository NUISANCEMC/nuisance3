// Copyright 2016-2021 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

/*******************************************************************************
 *    This file is part of NUISANCE.
 *
 *    NUISANCE is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    NUISANCE is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with NUISANCE.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#pragma once
#include <string>

#include "yaml-cpp/yaml.h"

using namespace YAML;

namespace nuis {
namespace measurement {

struct Document {
  std::string name;
  std::string data_file;
  std::string description;

  inline Document() {}

  inline explicit Document(YAML::Node node) {
    this->name.clear();
    this->data_file.clear();
    this->description.clear();

    // Return empty if no node
    if (!node) {
      return;
    }

    if (!node["name"]) return;

    if (node["name"]) this->name = node["name"].as<std::string>();
    if (node["data_file"]) {
      this->data_file = node["data_file"].as<std::string>();
    }
  }
};

}  // namespace measurement
}  // namespace nuis


// Allows node.as<Document>();
namespace YAML {
template <> struct convert<nuis::measurement::Document> {
  static bool decode(const Node &node, nuis::measurement::Document &rhs) {  // NOLINT
    rhs = nuis::measurement::Document(node);
    return true;
  }
};

};  // namespace YAML
