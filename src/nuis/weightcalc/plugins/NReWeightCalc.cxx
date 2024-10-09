#include "nuis/weightcalc/plugins/NReWeightCalc.h"

#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "CommonBlockIFace.h"
#include "NReWeightFactory.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include "nuis/except.h"
#include "nuis/log.txx"

#include <filesystem>

namespace nuis {

DECLARE_NUISANCE_EXCEPT(NEUTCardRequired);

double NReWeightCalc::calc_weight(HepMC3::GenEvent const &ev) {
  neut::CommonBlockIFace::ReadVect(nevs->neutvect(ev));
  auto sts = stop_talking_scopeguard();
  return fNEUTRW->CalcWeight();
}

void NReWeightCalc::set_parameters(
    std::map<std::string, double> const &params) {
  for (auto &[p, v] : params) {
    fNEUTRW->SetDial_To_Value(fNEUTRW->DialFromString(p), v);
  }
  auto sts = stop_talking_scopeguard();
  fNEUTRW->Reconfigure();
}

bool NReWeightCalc::good() const { return bool(nevs); }

NReWeightCalc::NReWeightCalc(IEventSourcePtr evs, YAML::Node const &cfg) {
  nevs = std::dynamic_pointer_cast<neutvectEventSource>(evs);
  if (!nevs) {
    log_warn("NReWeightCalc: Passed in IEventSourcePtr not instance of "
             "neutvectEventSource.");
    return;
  }
  if (cfg["neut_cardname"]) {
    neut::CommonBlockIFace::Initialize(cfg["neut_cardname"].as<std::string>(),
                                       false);
  } else {
    throw NEUTCardRequired()
        << "When using NReWeightCalc you must include the path to a value NEUT "
           "card file in the input YAML node via the attribute "
           "\"neut_cardname\".";
  }
  fNEUTRW = neut::rew::MakeNReWeightInstance();
}

NReWeightCalc::~NReWeightCalc() {}

IWeightCalcPluginPtr NReWeightCalc::MakeWeightCalc(IEventSourcePtr evs,
                                                   YAML::Node const &cfg) {
  return std::make_shared<NReWeightCalc>(evs, cfg);
}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::NReWeightCalc::MakeWeightCalc, MakeWeightCalc);
#endif

} // namespace nuis
