#pragma once

#include "nuis/weightcalc/IWeightCalc.h"

namespace HepMC3 {
class GenEvent;
}

namespace nuis {
template <typename ET, typename PT>
class WeightCalcFunc : public IWeightCalc<ET, PT> {

  using EvtType = ET;
  using ParamType = PT;

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