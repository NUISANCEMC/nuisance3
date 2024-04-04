#include "nuis/weightcalc/plugins/T2KReWeightCalc.h"

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "T2KReWeight/Interface/T2KReWeight.h"
#include "T2KReWeight/WeightEngines/NEUT/T2KNEUTUtils.h"
#include "T2KReWeight/WeightEngines/T2KReWeightEvent.h"
#include "T2KReWeight/WeightEngines/T2KReWeightFactory.h"

#include "boost/dll/alias.hpp"

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

double T2KReWeightCalc::calc_weight(HepMC3::GenEvent const &ev) {
  return fT2KRW->CalcWeight(t2krew::Event::Make(nevs->neutvect(ev)));
}

void T2KReWeightCalc::set_parameters(
    std::map<std::string, double> const &params) {
  for (auto &[p, v] : params) {
    fT2KRW->SetDial_To_Value(fT2KRW->DialFromString(p), v);
  }
  fT2KRW->Reconfigure();
}

bool T2KReWeightCalc::good() const { return bool(nevs); }

T2KReWeightCalc::T2KReWeightCalc(IEventSourcePtr evs, YAML::Node const &cfg) {
  nevs = std::dynamic_pointer_cast<neutvectEventSource>(evs);
  if (!nevs) {
    log_warn("T2KReWeightCalc: Passed in IEventSourcePtr not instance of "
             "neutvectEventSource.");
    return;
  }
  if (cfg["neut_cardname"]) {
    auto sts = stop_talking_scopeguard();
    t2krew::T2KNEUTUtils::SetCardFile(cfg["neut_cardname"].as<std::string>(),
                                      false);
  }
  auto sts = stop_talking_scopeguard();
  fT2KRW = t2krew::MakeT2KReWeightInstance(t2krew::Event::kNEUT);
}

T2KReWeightCalc::~T2KReWeightCalc() {}

namespace neut {
static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr evs,
                                           YAML::Node const &cfg) {
  return std::make_shared<T2KReWeightCalc>(evs, cfg);
}
} // namespace neut

BOOST_DLL_ALIAS(nuis::neut::MakeWeightCalc, MakeWeightCalc);

} // namespace nuis
