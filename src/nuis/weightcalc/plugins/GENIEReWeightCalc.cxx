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

#include "Framework/EventGen/GEVGDriver.h"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif

#include "nuis/log.txx"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

class GENIEReWeightCalc : public IWeightCalcPlugin {
  std::unique_ptr<genie::rew::GReWeight> fGENIE3RW;
  std::shared_ptr<GHEP3EventSource> nevs;

public:
  double calc_weight(HepMC3::GenEvent const &ev) {
    return fGENIE3RW->CalcWeight(*nevs->EventRecord(ev));
  };
  void set_parameters(std::map<std::string, double> const &params) {
    for (auto &[p, v] : params) {
      log_info("GENIEReWeightCalc: Setting parameter {} to {}", p, v);
      fGENIE3RW->Systematics().Set(GSyst::FromString(p), v);
    }
    log_info("GENIEReWeightCalc: Beginning Reconfigure");
    fGENIE3RW->Reconfigure();
    log_info("GENIEReWeightCalc: Done Reconfigure");
  };
  bool good() const { return bool(nevs); }

  GENIEReWeightCalc(IEventSourcePtr evs, YAML::Node const &) {
    nevs = std::dynamic_pointer_cast<GHEP3EventSource>(evs);
    if (!nevs) {
      log_warn(
          "GENIEReWeightCalc: Passed in IEventSourcePtr not instance of "
          "GHEP3EventSource.");
      return;
    }
    fGENIE3RW = std::make_unique<genie::rew::GReWeight>();

    fGENIE3RW->AdoptWghtCalc("xsec_ncel", new genie::rew::GReWeightNuXSecNCEL);
    fGENIE3RW->AdoptWghtCalc("xsec_ccqe", new genie::rew::GReWeightNuXSecCCQE);
    fGENIE3RW->AdoptWghtCalc("xsec_MEC", new genie::rew::GReWeightXSecMEC);
    fGENIE3RW->AdoptWghtCalc("xsec_coh", new genie::rew::GReWeightNuXSecCOH);
    fGENIE3RW->AdoptWghtCalc("xsec_nonresbkg",
                             new genie::rew::GReWeightNonResonanceBkg);
    fGENIE3RW->AdoptWghtCalc("nuclear_dis",
                             new genie::rew::GReWeightDISNuclMod);
    fGENIE3RW->AdoptWghtCalc("hadro_res_decay",
                             new genie::rew::GReWeightResonanceDecay);
    fGENIE3RW->AdoptWghtCalc("hadro_fzone", new genie::rew::GReWeightFZone);
    fGENIE3RW->AdoptWghtCalc("hadro_intranuke", new genie::rew::GReWeightINuke);
    fGENIE3RW->AdoptWghtCalc("hadro_agky", new genie::rew::GReWeightAGKY);
    fGENIE3RW->AdoptWghtCalc("xsec_ccqe_vec",
                             new genie::rew::GReWeightNuXSecCCQEvec);
    fGENIE3RW->AdoptWghtCalc("xsec_ccqe_axial",
                             new genie::rew::GReWeightNuXSecCCQEaxial);
    fGENIE3RW->AdoptWghtCalc("xsec_dis", new genie::rew::GReWeightNuXSecDIS);
    fGENIE3RW->AdoptWghtCalc("xsec_nc", new genie::rew::GReWeightNuXSecNC);
    fGENIE3RW->AdoptWghtCalc("xsec_ccres",
                             new genie::rew::GReWeightNuXSecCCRES);
    fGENIE3RW->AdoptWghtCalc("xsec_ncres",
                             new genie::rew::GReWeightNuXSecNCRES);
    fGENIE3RW->AdoptWghtCalc("nuclear_qe", new genie::rew::GReWeightFGM);
  };

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr evs,
                                             YAML::Node const &cfg) {
    return std::make_shared<GENIEReWeightCalc>(evs, cfg);
  }

GENIEReWeightCalc::~GENIEReWeightCalc() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::GENIEReWeightCalc::MakeWeightCalc, MakeWeightCalc);
#endif

} // namespace nuis
