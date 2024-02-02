#pragma once

#include "nuis/weightcalc/IWeightCalc.h"

#include <vector>

namespace nuis {

template <typename ET, typename PT>
class IWeightCalcContainer : public IWeightCalc<ET, PT> {
  using WCPtr = std::shared_ptr<IWeightCalc<ET, PT>>;
  std::vector<WCPtr> wcs;

public:
  using EvtType = ET;
  using ParamType = PT;

  IWeightCalcContainer(WCPtr wc = nullptr) {
    if (wc) {
      wcs.push_back(wc);
    }
  }
  void add(WCPtr wc) {
    if (wc) {
      wcs.push_back(wc);
    }
  }

  size_t size() { return wcs.size(); }

  // enum Policy { Skip, Zero, Unity, Include };

  // template <Policy NotNormalPolicy = Policy::Skip>
  double CalcWeight(EvtType const &ev) {
    double w = 1;
    for (auto &wc : wcs) {
      double wc_w = wc->CalcWeight(ev);
      bool isnorm = std::isnormal(wc_w);
      // if constexpr (NotNormalPolicy == Policy::Skip) {
      //   w *= isnorm ? wc_w : 1;
      // } else if (NotNormalPolicy == Policy::Zero) {
      //   return 0;
      // } else if (NotNormalPolicy == Policy::Unity) {
      //   return 1;
      // } else {
        w *= wc_w;
      // }
    }
    return w;
  }
  void SetParameters(ParamType const &p) {
    for (auto &wc : wcs) {
      wc->SetParameters(p);
    }
  }
};

using IWeightCalcContainerHM3Map =
    IWeightCalcContainer<HepMC3::GenEvent, std::map<std::string, double>>;

} // namespace nuis