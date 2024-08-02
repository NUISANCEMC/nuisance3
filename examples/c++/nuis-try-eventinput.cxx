#include "nuis/eventinput/EventSourceFactory.h"

#include "nuis/log.txx"

#include <iostream>
#include <memory>

int main(int argc, char const *argv[]) {

  if (argc != 2) {
    nuis::log_critical("Expected exactly 1 argument");
    return 1;
  }

  nuis::EventSourceFactory fact;

  std::string uri = argv[1];

  auto fe = uri.substr(uri.find_last_of('.') + 1, std::string::npos);

  if (fe == "root") {
    auto [gri, evs] = fact.make(uri);
    nuis::log_info(
        "After reading file: {}, have GenRunInfo: {}, EventSource: {}", uri,
        bool(gri), bool(evs));
    return !(gri && evs);
  } else {
    auto [gri, evs] = fact.make(YAML::LoadFile(uri));
    nuis::log_info(
        "After reading file: {}, have GenRunInfo: {}, EventSource: {}", uri,
        bool(gri), bool(evs));
    return !(gri && evs);
  }
}