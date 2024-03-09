#pragma once

#include "nuis/log.h"

#include <map>
#include <memory>
#include <string>

namespace HepMC3 {
class GenEvent;
}

namespace nuis {

template <typename ET, typename PT>
class IWeightCalc : public std::enable_shared_from_this<IWeightCalc<ET, PT>>,
                    public nuis_named_log("WeightCalc") {
public:
  using EvtType = ET;
  using ParamType = PT;

  virtual double calc_weight(EvtType const &) = 0;
  virtual void set_parameters(ParamType const &) = 0;

  // use to cast to known type
  template <typename T> std::shared_ptr<T> as() {
    return std::dynamic_pointer_cast<T>(this->shared_from_this());
  }
};

using IWeightCalcHM3Map =
    IWeightCalc<HepMC3::GenEvent, std::map<std::string, double>>;
using IWeightCalcHM3MapPtr = std::shared_ptr<IWeightCalcHM3Map>;

} // namespace nuis