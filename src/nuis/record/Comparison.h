#pragma once

#include "nuis/histframe/BinnedValues.h"

#include <functional>

namespace nuis {

struct Comparison {

  std::vector<BinnedValues> data;
  std::vector<BinnedValues> predictions;

  std::function<double()> likelihood;

};
} // namespace nuis
