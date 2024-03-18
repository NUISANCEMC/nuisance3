#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/ClearFunctions.h"
#include "nuis/record/FinalizeFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "nuis/frame/missing_datum.h"

#include "nuis/convert/ROOT.h"

#include "nuis/log.txx"

#include "boost/dll/alias.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>

// nuisance v2 headers]
#include "Measurement1D.h"
#include "Measurement2D.h"
#include "MeasurementBase.h"
#include "NuHepMCInputHandler.h"
#include "NuisConfig.h"
#include "SampleList.h"

namespace nuis {

class NUISANCERecord : public IRecordPlugin {
public:
  explicit NUISANCERecord(YAML::Node const &cfg) {
    node = cfg;

    Config::SetPar("EventManager", false);

    nuis2measurement = std::unique_ptr<MeasurementBase>(
        SampleUtils::CreateSample(node["name"].as<std::string>(),
                                  "Dummy:dummy.file", "", "", nullptr));

    // // Make an InputHandler
    nuis2inputhandler = std::make_unique<NuHepMCInputHandler>();
    nuis2fitevent = std::make_unique<FitEvent>();
    nuis2inputhandler->fNUISANCEEvent = nuis2fitevent.get();
    nuis2inputhandler->fNUISANCEEvent->HardReset();
    nuis2inputhandler->fToMeV = 1;
  }

  FitEvent *to_fit_event(HepMC3::GenEvent const &ev) {
    nuis2inputhandler->fHepMC3Evt = ev;
    nuis2inputhandler->CalcNUISANCEKinematics();
    return nuis2fitevent.get();
  }

  TablePtr table(std::string) {
    auto tbl = std::make_shared<Table>();

    BinningPtr binning;

    if (auto meas1d = dynamic_cast<Measurement1D *>(nuis2measurement.get())) {

      auto bv = BinnedValues_from_ROOT<TH1>(*meas1d->GetDataHistogram());
      tbl->blueprint = std::make_shared<Comparison>(bv.binning);
      tbl->blueprint->data = bv;

      tbl->project = [this](HepMC3::GenEvent const &ev) -> std::vector<double> {
        // allows us to call project on samples that may not be careful with
        // their projection functions
        auto fe = this->to_fit_event(ev);
        if (!nuis2measurement->isSignal(fe)) {
          return {nuis::kMissingDatum<double>};
        }
        this->nuis2measurement->FillEventVariables(fe);
        return {this->nuis2measurement->GetXVar()};
      };

    } else if (auto meas2d =
                   dynamic_cast<Measurement2D *>(nuis2measurement.get())) {
      auto bv = BinnedValues_from_ROOT<TH2>(*meas2d->GetDataHistogram());
      tbl->blueprint = std::make_shared<Comparison>(bv.binning);
      tbl->blueprint->data = bv;

      tbl->project = [this](HepMC3::GenEvent const &ev) -> std::vector<double> {
        // allows us to call project on samples that may not be careful with
        // their projection functions
        auto fe = this->to_fit_event(ev);
        if (!nuis2measurement->isSignal(fe)) {
          return {nuis::kMissingDatum<double>, nuis::kMissingDatum<double>};
        }
        this->nuis2measurement->FillEventVariables(fe);
        return {this->nuis2measurement->GetXVar(),
                this->nuis2measurement->GetYVar()};
      };
    }

    tbl->clear = nuis::clear::DefaultClear;
    tbl->weight = nuis::weight::DefaultWeight;
    tbl->finalize = nuis::finalize::FATXNormalizedByBinWidth;
    tbl->likeihood = nuis::likelihood::Chi2;

    tbl->select = [&](HepMC3::GenEvent const &ev) -> int {
      return nuis2measurement->isSignal(this->to_fit_event(ev));
    };

    return tbl;
  }

  bool good() const { return true; }

  static IRecordPluginPtr Make(YAML::Node const &cfg) {
    return std::make_shared<NUISANCERecord>(cfg);
  }

  virtual ~NUISANCERecord() {}

  std::unique_ptr<MeasurementBase> nuis2measurement;
  std::unique_ptr<FitEvent> nuis2fitevent;
  std::unique_ptr<NuHepMCInputHandler> nuis2inputhandler;
};

BOOST_DLL_ALIAS(nuis::NUISANCERecord::Make, Make);

} // namespace nuis
