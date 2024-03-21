#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/ClearFunctions.h"
#include "nuis/record/FinalizeFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "nuis/eventframe/missing_datum.h"

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
  // this RAII's the NUISANCE2 measurement instance and we store value copies of
  // instances of this in the select and project lambda's captures. Everything
  // is declared mutable because lambdas are by default const wrt value captures
  //
  // It's a hack, but this whole class is a hack
  struct nuis2mblob {
    mutable std::shared_ptr<MeasurementBase> measurement;
    mutable std::shared_ptr<NuHepMCInputHandler> inputhandler;
    mutable std::shared_ptr<FitEvent> fitevent;

    FitEvent *to_fit_event(HepMC3::GenEvent const &ev) const {
      inputhandler->fHepMC3Evt = ev;
      inputhandler->CalcNUISANCEKinematics();
      return fitevent.get();
    }

    template <typename T> T *measurement_as() const {
      return dynamic_cast<T *>(measurement.get());
    }
  };

  explicit NUISANCERecord(YAML::Node const &cfg) { node = cfg; }

  TablePtr table(std::string const &name) {
    auto tbl = std::make_shared<Table>();

    Config::SetPar("EventManager", false);
    Config::SetPar("UseSVDInverse", true);

    nuis2mblob mblob{std::shared_ptr<MeasurementBase>(SampleUtils::CreateSample(
                         name, "Dummy:dummy.file", "", "", nullptr)),
                     std::make_shared<NuHepMCInputHandler>(),
                     std::make_shared<FitEvent>()};
    mblob.inputhandler->fNUISANCEEvent = mblob.fitevent.get();
    mblob.inputhandler->fNUISANCEEvent->HardReset();
    mblob.inputhandler->fToMeV = 1;

    BinningPtr binning;

    if (auto meas1d = mblob.measurement_as<Measurement1D>()) {

      auto bv = BinnedValues_from_ROOT<TH1>(*meas1d->GetDataHistogram());
      tbl->blueprint = std::make_shared<Comparison>(bv.binning);
      tbl->blueprint->data = bv;

    } else if (auto meas2d = mblob.measurement_as<Measurement2D>()) {
      auto bv = BinnedValues_from_ROOT<TH2>(*meas2d->GetDataHistogram());
      tbl->blueprint = std::make_shared<Comparison>(bv.binning);
      tbl->blueprint->data = bv;
    }

    tbl->clear = nuis::clear::DefaultClear;
    tbl->weight = nuis::weight::DefaultWeight;
    tbl->finalize = nuis::finalize::FATXNormalizedByBinWidth;
    tbl->likeihood = nuis::likelihood::Chi2;

    tbl->select = [mblob](HepMC3::GenEvent const &ev) -> int {
      return mblob.measurement->isSignal(mblob.to_fit_event(ev));
    };

    // always return a vector of three, it is up to users to know how many they
    // need to listen for
    tbl->project = [mblob](HepMC3::GenEvent const &ev) -> std::vector<double> {
      // allows us to call project on samples that may not be careful with
      // their projection functions
      auto fe = mblob.to_fit_event(ev);
      if (!mblob.measurement->isSignal(fe)) {
        return {nuis::kMissingDatum<double>, nuis::kMissingDatum<double>,
                nuis::kMissingDatum<double>};
      }
      mblob.measurement->FillEventVariables(fe);
      return {mblob.measurement->GetXVar(), mblob.measurement->GetYVar(),
              mblob.measurement->GetZVar()};
    };

    return tbl;
  }

  bool good() const { return true; }

  static IRecordPluginPtr Make(YAML::Node const &cfg) {
    return std::make_shared<NUISANCERecord>(cfg);
  }

  virtual ~NUISANCERecord() {}
};

BOOST_DLL_ALIAS(nuis::NUISANCERecord::Make, Make);

} // namespace nuis
