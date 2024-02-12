
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

#include <pybind11/pybind11.h>

#include "ProSelecta/ProSelecta.h"

std::vector<std::string> include_paths;
std::string ProSelecta_env_dir;

namespace py = pybind11;

void configure_environment() {

    char *ProSelecta_env = getenv("PROSELECTA_DIR");
    if (!ProSelecta_env && !ProSelecta_env_dir.length()) {
        std::cout <<
            "[ERROR]: Cannot find ProSelecta environment headers. Either "
            "define PROSELECTA_DIR in the calling environment or add "
            "--env command line argument."
            << std::endl;

        return;
    }

    if (!ProSelecta_env_dir.length()) {
        ProSelecta_env_dir = ProSelecta_env;
        if (ProSelecta_env_dir.back() != '/') {
        ProSelecta_env_dir += '/';
        }
        ProSelecta_env_dir += "ProSelecta/env/";
    }

    ProSelecta::Get().AddIncludePath(ProSelecta_env_dir);

    bool read_env = ProSelecta::Get().LoadText("#include \"env.h\"",
                        ProSelecta::Interpreter::kCling);

    if (!read_env) {
        std::cout
            << "[ERROR]: Cling failed to interpret the processor environment, if "
            "you passed the right path to find these header files and this "
            "still occures then it is a bug in ProSelectaCPP itself."
            << std::endl;
        abort();
    }

    // Should also check it exists
    auto DATABASE = getenv("NUISANCEDB");
    if (!DATABASE){
        std::cout
            << "[ERROR]: NUISANCEDB NOT SET!" << std::endl;
        abort();
    }
}



PYBIND11_MODULE(pyNuisAnalysis, m) {
    m.doc() = "NUISANCE implementation in python";
    m.def("configure", &configure_environment);
}
