#pragma once

#include <map>
#include <memory>
#include <string>

namespace HepMC3 {
class GenEvent;
}

namespace nuis {

template <typename ET, typename PT> class IWeightCalc {
public:
  using EvtType = ET;
  using ParamType = PT;

  virtual double CalcWeight(EvtType const &) = 0;
  virtual void SetParameters(ParamType const &) = 0;
};

using IWeightCalcHM3Map =
    IWeightCalc<HepMC3::GenEvent, std::map<std::string, double>>;
using IWeightCalcHM3MapPtr = std::shared_ptr<IWeightCalcHM3Map>;

} // namespace nuis