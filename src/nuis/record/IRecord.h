#pragma once

#include <string>
#include <memory>

#include "yaml-cpp/yaml.h"

#include "ProSelecta/ProSelecta.h"

#include "nuis/record/Table.h"

namespace nuis {

using TablePtr = std::shared_ptr<Table>;

struct IRecord {

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