#include "NuHepMC/make_writer.hxx"

#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/log.txx"

#include "fmt/ranges.h"

#include <iostream>

std::vector<std::string> files_to_read;
std::string file_to_write;
std::string plugin_name;

void SayUsage(char const *argv[]) {
  std::cout << "[USAGE]: " << argv[0] << "\n"
            << "\t-i <input1> [input2 ...]  : neutvect file to read\n"
            << "\t-o <neut.hepmc3>             : hepmc3 file to write\n"
            << "\t-p <pluginname>              : The name of the reader plugin "
               "to use\n"
            << std::endl;
}

void handleOpts(int argc, char const *argv[]) {
  int opt = 1;
  while (opt < argc) {
    if (std::string(argv[opt]) == "-?" || std::string(argv[opt]) == "--help") {
      SayUsage(argv);
      exit(0);
    } else if ((opt + 1) < argc) {
      if (std::string(argv[opt]) == "-i") {
        while (((opt + 1) < argc) && (argv[opt + 1][0] != '-')) {
          files_to_read.push_back(argv[++opt]);
          std::cout << "[INFO]: Reading from " << files_to_read.back()
                    << std::endl;
        }
      } else if (std::string(argv[opt]) == "-o") {
        file_to_write = argv[++opt];
      } else if (std::string(argv[opt]) == "-p") {
        plugin_name = argv[++opt];
        std::cout << "[INFO]: Reading with plugin " << plugin_name << std::endl;
      } else {
        std::cout << "[ERROR]: Unknown option: " << argv[opt] << std::endl;
        SayUsage(argv);
        exit(1);
      }
    } else {
      std::cout << "[ERROR]: Unknown option: " << argv[opt] << std::endl;
      SayUsage(argv);
      exit(1);
    }
    opt++;
  }
}

int main(int argc, char const *argv[]) {

  handleOpts(argc, argv);

  if (!files_to_read.size() || !file_to_write.length()) {
    nuis::log_critical("[ERROR]: Expected -i and -o arguments.");
    return 1;
  }

  YAML::Node cfg;
  if (plugin_name.size()) {
    cfg["plugin_name"] = plugin_name;
  }
  cfg["filepaths"] = files_to_read;

  nuis::EventSourceFactory fact;
  auto [gri, evs] = fact.make(cfg);

  if (!evs) {
    nuis::log_critical("Failed to find EventSource for input files {}",
                       files_to_read);
    return 1;
  }

  auto wrtr = NuHepMC::Writer::make_writer(file_to_write, gri);
  for (auto [ev, cvw] : evs) {
    ev->set_run_info(gri);
    wrtr->write_event(*ev);
  }
  wrtr->close();
}