#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/record/Comparison.h"

namespace nuis {

using ProjectFunc = std::function<double(HepMC3::GenEvent const &)>;
using SelectFunc = std::function<int(HepMC3::GenEvent const &)>;
using WeightFunc = std::function<double(HepMC3::GenEvent const &)>;
using FinalizeFunc = std::function<BinnedValues(HistFrame &, const double)>;
using LikelihoodFunc = std::function<double(Comparison const &)>;

struct IAnalysis {

  NEW_NUISANCE_EXCEPT(IAnalysisUnimplementedInterfaceFunction);

  virtual Comparison process(std::vector<INormalizedEventSourcePtr> &) {
    throw IAnalysisUnimplementedInterfaceFunction();
  };
  virtual Comparison process(INormalizedEventSourcePtr &) {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::string, SelectFunc> get_selection() {
    throw IAnalysisUnimplementedInterfaceFunction();
  };
  virtual std::pair<std::vector<std::string>, std::vector<SelectFunc>>
  get_all_selections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  };
  virtual std::pair<std::vector<std::vector<std::string>>,
                    std::vector<std::vector<ProjectFunc>>>
  get_all_projections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  virtual std::vector<BinnedValues> get_data() {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  WeightFunc weight;
  FinalizeFunc finalise;
  LikelihoodFunc likelihood;

  virtual std::string
  prediction_generation_hint(std::string const &) {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  virtual ~IAnalysis() {};
};

using AnalysisPtr = std::shared_ptr<IAnalysis>;

} // namespace nuis
