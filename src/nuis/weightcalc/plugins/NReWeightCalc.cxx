#include "nuis/weightcalc/plugins/IWeightCalcPlugin.h"

#include "nuis/eventinput/plugins/neutvectEventSource.h"

#include "CommonBlockIFace.h"
#include "NReWeightFactory.h"

#include "boost/dll/alias.hpp"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace nuis {

class NReWeightCalc : public IWeightCalcPlugin {
  std::unique_ptr<neut::rew::NReWeight> fNEUTRW;
  std::shared_ptr<neutvectEventSource> nevs;

public:
  double calc_weight(HepMC3::GenEvent const &ev) {
    neut::CommonBlockIFace::ReadVect(nevs->neutvect(ev));
    return fNEUTRW->CalcWeight();
  };
  void set_parameters(std::map<std::string, double> const &params) {
    for (auto &[p, v] : params) {
      fNEUTRW->SetDial_To_Value(fNEUTRW->DialFromString(p), v);
    }
    fNEUTRW->Reconfigure();
  };
  bool good() const { return bool(nevs); }

  NReWeightCalc(IEventSourcePtr evs, YAML::Node const &cfg) {
    nevs = std::dynamic_pointer_cast<neutvectEventSource>(evs);
    if (!nevs) {
      spdlog::warn("NReWeightCalc: Passed in IEventSourcePtr not instance of "
                   "neutvectEventSource.");
      return;
    }
    neut::CommonBlockIFace::Initialize(cfg["neut_cardname"].as<std::string>(),
                                       false);
    fNEUTRW = neut::rew::MakeNReWeightInstance();
  };

  static IWeightCalcPluginPtr MakeWeightCalc(IEventSourcePtr evs,
                                                       YAML::Node const &cfg) {
    return std::make_shared<NReWeightCalc>(evs, cfg);
  }

  virtual ~NReWeightCalc() {}
};

BOOST_DLL_ALIAS(nuis::NReWeightCalc::MakeWeightCalc,
                MakeWeightCalc);

} // namespace nuis
