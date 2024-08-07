#pragma once

#include "nuis/except.h"

#include <string>

namespace nuis {

NEW_NUISANCE_EXCEPT(EnvVarNotDefined);

namespace env {

#define NEW_NUISANCE_ENVVAR(A)                                                 \
  std::string A() {                                                                   \
    auto A_val = std::getenv(#A);                                              \
    if (!A_val) {                                                              \
      throw EnvVarNotDefined() << "Environment variable: " << #A               \
                               << " was requested, but is not defined.";       \
    }                                                                          \
    return A_val;                                                              \
  }                                                                            \
  std::string A(std::string const &default_val) {                                     \
    auto A_val = std::getenv(#A);                                              \
    if (!A_val) {                                                              \
      return default_val;                                                      \
    }                                                                          \
    return A_val;                                                              \
  }

NEW_NUISANCE_ENVVAR(NUISANCE3_ROOT);
NEW_NUISANCE_ENVVAR(NUISANCEDB);
NEW_NUISANCE_ENVVAR(NUISANCE_EVENT_PATH);

#undef NEW_NUISANCE_ENVVAR

} // namespace env

} // namespace nuis