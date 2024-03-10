#pragma once

#include "nuis/histframe/HistFrame.h"

#include "yaml-cpp/yaml.h"

#include "boost/json.hpp"

namespace nuis {

namespace plotly {
std::string to_1D_json(HistFrame const &hf);
}

namespace matplotlib {
std::map<std::string, Eigen::ArrayXXd>
to_pcolormesh_data(HistFrame const &hf, HistFrame::column_t colid = 0);
}

YAML::Node to_yaml(HistFrame const &hf);
std::string to_yaml_str(HistFrame const &hf);

HistFrame from_yaml(YAML::Node const &yhf);
HistFrame from_yaml_str(std::string const &shf);

// boost::json overloads
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                Binning const &bi);

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                HistFrame const &hf);
} // namespace nuis