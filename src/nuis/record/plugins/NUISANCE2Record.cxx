#include "nuis/record/plugins/NUISANCE2Record.h"

#include "nuis/record/plugins/IRecordPlugin.h"

#include "nuis/record/FinaliseFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "nuis/eventframe/missing_datum.h"

#include "nuis/convert/ROOT.h"

#include "nuis/log.txx"

#ifdef NUISANCE_USE_BOOSTDLL
#include "boost/dll/alias.hpp"
#endif
#include <filesystem>

// nuisance v2 headers
#include "Measurement1D.h"
#include "Measurement2D.h"
#include "MeasurementBase.h"
#include "NuHepMCInputHandler.h"
#include "NuisConfig.h"
#include "SampleList.h"

namespace nuis {

DECLARE_NUISANCE_EXCEPT(NUISANCE2InvalidMeasurementType);

struct NUISANCE2Analysis : public IAnalysis {

  // the IAnalysis interface uses consts, the nuisance2 one doesn't
  mutable std::shared_ptr<MeasurementBase> measurement;
  mutable std::shared_ptr<NuHepMCInputHandler> inputhandler;
  mutable std::shared_ptr<FitEvent> fitevent;

  NUISANCE2Analysis(std::string const &ana_name) {
    Config::SetPar("EventManager", false);
    Config::SetPar("UseSVDInverse", true);

    measurement = std::shared_ptr<MeasurementBase>(SampleUtils::CreateSample(
        ana_name, "Dummy:dummy.file", "", "", nullptr));
    inputhandler = std::make_shared<NuHepMCInputHandler>();
    fitevent = std::make_shared<FitEvent>();

    inputhandler->fNUISANCEEvent = fitevent.get();
    inputhandler->fNUISANCEEvent->HardReset();
    inputhandler->fToMeV = 1;
  }

  FitEvent *to_fit_event(HepMC3::GenEvent const &ev) const {
    inputhandler->fHepMC3Evt = ev;
    inputhandler->CalcNUISANCEKinematics();
    return fitevent.get();
  }

  void add_to_framegen(EventFrameGen &) const {}

  Selection get_selection() const {
    // keeping a copy of the selection function around keeps a copy of the
    // nuisance2 analysis around
    auto ana =
        std::dynamic_pointer_cast<NUISANCE2Analysis const>(shared_from_this());
    return {"MeasurementBase::IsSignal",
            [=](HepMC3::GenEvent const &ev) -> int {
              return ana->measurement->isSignal(ana->to_fit_event(ev));
            }};
  }

  std::vector<Projection> get_projections() const {

    auto ana =
        std::dynamic_pointer_cast<NUISANCE2Analysis const>(shared_from_this());

    if (auto meas1d = std::dynamic_pointer_cast<Measurement1D>(measurement)) {
      return std::vector<Projection>{
          {"XVar",
           [=](HepMC3::GenEvent const &ev) -> double {
             auto fe = ana->to_fit_event(ev);
             if (!ana->measurement->isSignal(fe)) {
               return nuis::kMissingDatum<double>;
             }
             ana->measurement->FillEventVariables(fe);
             return ana->measurement->GetXVar();
           },
           "", ""}};

    } else if (auto meas2d =
                   std::dynamic_pointer_cast<Measurement2D>(measurement)) {
      return std::vector<Projection>{
          {"MeasurementBase::GetXVar",
           [=](HepMC3::GenEvent const &ev) -> double {
             auto fe = ana->to_fit_event(ev);
             if (!ana->measurement->isSignal(fe)) {
               return nuis::kMissingDatum<double>;
             }
             ana->measurement->FillEventVariables(fe);
             return ana->measurement->GetXVar();
           },
           "", ""},
          {"MeasurementBase::GetYVar",
           [=](HepMC3::GenEvent const &ev) -> double {
             auto fe = ana->to_fit_event(ev);
             if (!ana->measurement->isSignal(fe)) {
               return nuis::kMissingDatum<double>;
             }
             ana->measurement->FillEventVariables(fe);
             return ana->measurement->GetYVar();
           },
           "", ""}};
    }
    throw NUISANCE2InvalidMeasurementType();
  }
  std::vector<BinnedValues> get_data() const {
    if (auto meas1d = std::dynamic_pointer_cast<Measurement1D>(measurement)) {
      return std::vector<BinnedValues>{
          BinnedValues_from_ROOT<TH1>(*meas1d->GetDataHistogram())};
    } else if (auto meas2d =
                   std::dynamic_pointer_cast<Measurement2D>(measurement)) {
      return std::vector<BinnedValues>{
          BinnedValues_from_ROOT<TH2>(*meas2d->GetDataHistogram())};
    }
    throw NUISANCE2InvalidMeasurementType();
  }

  Eigen::MatrixXd get_covariance_matrix() const {
    TMatrixDSym *fFullCovar = nullptr;

    if (auto meas1d = std::dynamic_pointer_cast<Measurement1D>(measurement)) {
      fFullCovar = meas1d->fFullCovar;
    } else if (auto meas2d =
                   std::dynamic_pointer_cast<Measurement2D>(measurement)) {
      fFullCovar = meas2d->fFullCovar;
    }

    if (!fFullCovar) {
      return Eigen::MatrixXd{};
    }

    Eigen::MatrixXd covmat =
        Eigen::MatrixXd::Zero(fFullCovar->GetNrows(), fFullCovar->GetNrows());
    for (int i = 0; i < fFullCovar->GetNrows(); ++i) {
      for (int j = 0; j < fFullCovar->GetNrows(); ++j) {
        covmat(i, j) = (*fFullCovar)(i, j);
      }
    }
    return covmat;
  }
};

NUISANCE2Record::NUISANCE2Record(YAML::Node const &cfg) { node = cfg; }

AnalysisPtr NUISANCE2Record::analysis(YAML::Node const &cfg) {

  auto ana_name = cfg["analysis"].as<std::string>();

  log_critical("NUISANCE2Record::analysis({})", ana_name);

  if (!analyses.count(ana_name)) {
    analyses[ana_name] = std::make_shared<NUISANCE2Analysis>(ana_name);
  }

  return analyses[ana_name];
}

IRecordPluginPtr NUISANCE2Record::MakeRecord(YAML::Node const &cfg) {
  return std::make_unique<NUISANCE2Record>(cfg);
}

NUISANCE2Record::~NUISANCE2Record() {}

#ifdef NUISANCE_USE_BOOSTDLL
BOOST_DLL_ALIAS(nuis::NUISANCE2Record::Make, Make);
#endif

} // namespace nuis
