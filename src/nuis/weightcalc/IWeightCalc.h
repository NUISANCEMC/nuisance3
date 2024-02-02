#pragma once

namespace nuis {

template <typename ET, typename PT> class IWeightCalc {
public:
  virtual double operator()(ET const &) = 0;
  virtual void SetParameters(PT const &) = 0;
};

} // namespace nuis