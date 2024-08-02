#include "nuis/weightcalc/plugins/GENIEReWeightCalc.h"

#include "RwFramework/GReWeight.h"

#include "RwCalculators/GReWeightAGKY.h"
#include "RwCalculators/GReWeightDISNuclMod.h"
#include "RwCalculators/GReWeightFGM.h"
#include "RwCalculators/GReWeightFZone.h"
#include "RwCalculators/GReWeightINuke.h"
#include "RwCalculators/GReWeightNonResonanceBkg.h"
#include "RwCalculators/GReWeightNuXSecCCQE.h"
#include "RwCalculators/GReWeightNuXSecCCQEaxial.h"
#include "RwCalculators/GReWeightNuXSecCCQEvec.h"
#include "RwCalculators/GReWeightNuXSecCCRES.h"
#include "RwCalculators/GReWeightNuXSecCOH.h"
#include "RwCalculators/GReWeightNuXSecDIS.h"
#include "RwCalculators/GReWeightNuXSecNC.h"
#include "RwCalculators/GReWeightNuXSecNCEL.h"
#include "RwCalculators/GReWeightNuXSecNCRES.h"
#include "RwCalculators/GReWeightResonanceDecay.h"
#include "RwCalculators/GReWeightXSecEmpiricalMEC.h"
#include "RwCalculators/GReWeightXSecMEC.h"
#if __has_include("RwCalculators/GReWeightNuXSecCCQEELFF.h")
#include "RwCalculators/GReWeightNuXSecCCQEELFF.h"
#define GRW_HAS_GReWeightNuXSecCCQEELFF
#endif

#include "Framework/EventGen/GEVGDriver.h"
#include "Framework/Messenger/Messenger.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

namespace nuis {

double GENIEReWeightCalc::calc_weight(HepMC3::GenEvent const &ev) {
  return fGENIE3RW->CalcWeight(*nevs->EventRecord(ev));
}
void GENIEReWeightCalc::set_parameters(
    std::map<std::string, double> const &params) {

  genie::Messenger::Instance()->SetPriorityLevel("ReW", pERROR);

  for (auto &[p, v] : params) {
    log_info("GENIEReWeightCalc: Setting parameter {} to {}", p, v);
    fGENIE3RW->Systematics().Set(GSyst::FromString(p), v);
  }
  log_info("GENIEReWeightCalc: Beginning Reconfigure");
  fGENIE3RW->Reconfigure();
  log_info("GENIEReWeightCalc: Done Reconfigure");
}

GENIEReWeightCalc::GENIEReWeightCalc(IEventSourcePtr evs,
                                     YAML::Node const &cfg) {
  nevs = std::dynamic_pointer_cast<GHEP3EventSource>(evs);
  if (!nevs) {
    log_warn("GENIEReWeightCalc: Passed in IEventSourcePtr not instance of "
             "GHEP3EventSource.");
    return;
  }
  fGENIE3RW = std::make_unique<genie::rew::GReWeight>();

  auto adopt_engines = cfg["adopt"]
                           ? cfg["adopt"].as<std::vector<std::string>>()
                           : std::vector<std::string>{};
  auto all_on =
      cfg["adopt_all"] ? cfg["adopt_all"].as<bool>() : !adopt_engines.size();

#define NUIS_LIST                                                              \
  X(GReWeightNuXSecNCEL)                                                       \
  X(GReWeightNuXSecCCQE)                                                       \
  X(GReWeightXSecMEC)                                                          \
  X(GReWeightNuXSecCOH)                                                        \
  X(GReWeightNonResonanceBkg)                                                  \
  X(GReWeightDISNuclMod)                                                       \
  X(GReWeightResonanceDecay)                                                   \
  X(GReWeightFZone)                                                            \
  X(GReWeightINuke)                                                            \
  X(GReWeightAGKY)                                                             \
  X(GReWeightNuXSecCCQEvec)                                                    \
  X(GReWeightNuXSecCCQEaxial)                                                  \
  X(GReWeightNuXSecDIS)                                                        \
  X(GReWeightNuXSecNC)                                                         \
  X(GReWeightNuXSecCCRES)                                                      \
  X(GReWeightNuXSecNCRES)                                                      \
  X(GReWeightFGM)

#define X(RWTYPE)                                                              \
  if (all_on || std::find(adopt_engines.begin(), adopt_engines.end(),          \
                          #RWTYPE) != adopt_engines.end()) {                   \
    fGENIE3RW->AdoptWghtCalc(#RWTYPE, new genie::rew::RWTYPE);                 \
  }

  NUIS_LIST

#undef NUIS_LIST
#undef X

#ifdef GRW_HAS_GReWeightNuXSecCCQEELFF
  if (all_on || std::find(adopt_engines.begin(), adopt_engines.end(),
                          "GReWeightNuXSecCCQEELFF") != adopt_engines.end()) {
    fGENIE3RW->AdoptWghtCalc("GReWeightNuXSecCCQEELFF",
                             new genie::rew::GReWeightNuXSecCCQEELFF);
  }
#endif
}

IWeightCalcPluginPtr GENIEReWeightCalc::MakeWeightCalc(IEventSourcePtr evs,
                                                       YAML::Node const &cfg) {
  return std::make_shared<GENIEReWeightCalc>(evs, cfg);
}

GENIEReWeightCalc::~GENIEReWeightCalc() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::GENIEReWeightCalc::MakeWeightCalc, MakeWeightCalc);
#endif

} // namespace nuis
