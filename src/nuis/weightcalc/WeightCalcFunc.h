#pragma once

#include "nuis/weightcalc/IWeightCalc.h"

#include <functional>
#include <vector>

namespace nuis {
template <typename EvtType, typename ParamType>
class WeightCalcFunc : public IWeightCalc<EvtType, ParamType> {

  using FuncType = double(EvtType const &, ParamType const &);

  std::function<FuncType> func;
  ParamType params;

public:
  WeightCalcFunc(std::function<FuncType> f) : func(f) {}

  double CalcWeight(EvtType const &evt) { return func(evt, params); }
  void SetParameters(ParamType const &p) { params = p; }
};

using WeightCalcFuncHM3 = WeightCalcFunc<HepMC3::GenEvent, std::vector<double>>;
using WeightCalcFuncHM3Map =
    WeightCalcFunc<HepMC3::GenEvent, std::map<std::string, double>>;

} // namespace nuis