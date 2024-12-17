#include "nuis/weightcalc/plugins/nusystematicsWeightCalc.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include "nuis/except.h"
#include "nuis/log.txx"

#include <filesystem>

namespace nuis {

DECLARE_NUISANCE_EXCEPT(NoParamHeaders);

double nusystematicsWeightCalc::calc_weight(HepMC3::GenEvent const &ev) {

  if (!splines.count(ev.event_number())) {
    auto evr = nusyst->GetEventResponses(*nevs->EventRecord(ev));
    systtools::param_list_t params;
    for (auto const &[p, r] : evr) {
      params.push_back(p);
    }
    splines[ev.event_number()] = nusyst->GetSplines(params, evr);
  }

  double weight = 1;
  auto &espl = splines[ev.event_number()];
  for (auto const &[pid, v] : set_params) {
    if (!espl.count(pid)) {
      continue;
    }
    weight *= espl[pid].Eval(v);
  }
  return weight;
}

void nusystematicsWeightCalc::set_parameters(
    std::map<std::string, double> const &params) {
  for (auto &[p, v] : params) {
    systtools::paramId_t pid = nusyst->GetHeaderId(p);
    auto cid = systtools::GetParamContainerIndex(set_params, pid);
    if (cid == systtools::kParamUnhandled<size_t>) {
      set_params.push_back(systtools::ParamValue{pid, v});
    } else {
      set_params[cid].val = v;
    }
  }
}

bool nusystematicsWeightCalc::good() const { return bool(nevs); }

nusystematicsWeightCalc::nusystematicsWeightCalc(IEventSourcePtr evs,
                                                 YAML::Node const &cfg) {
  nevs = std::dynamic_pointer_cast<GHEP3EventSource>(evs);
  if (!nevs) {
    log_warn(
        "nusystematicsWeightCalc: Passed in IEventSourcePtr not instance of "
        "GHEP3EventSource.");
    return;
  }
  if (!cfg["param_headers"]) {
    throw NoParamHeaders()
        << "nusystematicsWeightCalc: On initialisation, no param_headers key "
           "found in input configuration node.";
  }
  std::filesystem::path param_headers = cfg["param_headers"].as<std::string>();
  if (!std::filesystem::exists(param_headers)) {
    throw NoParamHeaders()
        << "nusystematicsWeightCalc: On initialisation, param_headers value: "
        << param_headers.native() << " does not point to an existing file.";
  }

  nusyst = std::make_unique<nusyst::response_helper>(param_headers.native());
}

nusystematicsWeightCalc::~nusystematicsWeightCalc() {}

IWeightCalcPluginPtr
nusystematicsWeightCalc::MakeWeightCalc(IEventSourcePtr evs,
                                        YAML::Node const &cfg) {
  return std::make_shared<nusystematicsWeightCalc>(evs, cfg);
}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::nusystematicsWeightCalc::MakeWeightCalc, MakeWeightCalc);
#endif

} // namespace nuis
