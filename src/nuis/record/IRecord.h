#pragma once

#include <string>
#include <memory>

#include "yaml-cpp/yaml.h"

#include "ProSelecta/ProSelecta.h"

#include "nuis/record/Table.h"

#include "nuis/log.h"

namespace nuis {

using TablePtr = std::shared_ptr<Table>;

struct IRecord : public nuis_named_log("Record") {

    IRecord(){};

    IRecord(YAML::Node /*n*/){
        std::cout << "Base constructor being called" << std::endl;
    };

    virtual TablePtr table(std::string name) = 0;

    TablePtr operator[](std::string name) {
        return table(name);
    }

    YAML::Node node;

};
}