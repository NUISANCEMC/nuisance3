#pragma once

#include "nuis/weightcalc/IWeightCalc.h"

#include <memory>

namespace nuis {
class IWeightCalcPlugin : public IWeightCalcHM3Map {
public:
  virtual bool good() const = 0;
};

using IWeightCalcPluginPtr = std::shared_ptr<IWeightCalcPlugin>;
} // namespace nuis