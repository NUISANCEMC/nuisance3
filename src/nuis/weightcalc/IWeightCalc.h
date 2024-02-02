#pragma once

namespace nuis {

template <typename EvtType, typename ParamType> class IWeightCalc {
public:
  virtual double operator()(EvtType const &) = 0;
  virtual void SetParameters(ParamType const &) = 0;
};

} // namespace nuis