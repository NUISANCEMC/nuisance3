#pragma once

#include "nuis/weightcalc/IWeightCalc.h"

namespace HepMC3 {
class GenEvent;
}

namespace nuis {
template <typename EvtType, typename ParamType>
class WeightCalcFunc : public IWeightCalc<EvtType, ParamType> {

  using FuncType = double(EvtType const &, ParamType const &);

  std::function<FuncType> func;
  ParamType params;

public:
  WeightCalcFunc(FuncType f) : func(f) {}

  double operator()(EvtType const &evt) { return func(evt, params); }
  void SetParameters(ParamType const &p) { params = p; }
};

using WeightCalcFuncHM3 = WeightCalcFunc<HepMC3::GenEvent, std::vector<double>>;
using WeightCalcFuncHM3_NamedParams =
    WeightCalcFunc<HepMC3::GenEvent, std::map<std::string, double>>;

} // namespace nuis