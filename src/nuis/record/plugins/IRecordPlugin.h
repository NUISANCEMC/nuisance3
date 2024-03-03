#pragma once

#include "nuis/record/IRecord.h"

#include <memory>

namespace nuis {
class IRecordPlugin : public IRecord {
 public:
  virtual bool good() const = 0;
};

using IRecordPluginPtr = std::shared_ptr<IRecordPlugin>;
} // namespace nuis
