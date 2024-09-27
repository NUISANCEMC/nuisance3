#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/record/Comparison.h"

#include "NuHepMC/UnitsUtils.hxx"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

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
  }
  virtual Comparison process(INormalizedEventSourcePtr &) {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::string, SelectFunc> get_selection() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }
  virtual std::pair<std::vector<std::string>, std::vector<SelectFunc>>
  get_all_selections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }
  virtual std::pair<std::vector<std::vector<std::string>>,
                    std::vector<std::vector<ProjectFunc>>>
  get_all_projections() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual std::vector<BinnedValues> get_data() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual std::pair<int, BinnedValues> get_probe_flux(bool = false) {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual Eigen::MatrixXd get_covariance_matrix() const {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual Eigen::MatrixXd get_correlation_matrix() const {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual Eigen::MatrixXd get_smearing_matrix() const {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual std::vector<std::pair<int, BinnedValues>>
  get_all_probe_fluxes(bool = false) {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  struct Target {
    Target(std::pair<double, double> const &target_spec,
           double mass_weight = 1) {
      A = target_spec.first;
      Z = target_spec.second;
      N = A - Z;

      weight_by_mass = mass_weight;
    }
    double A, N, Z;
    double weight_by_mass;
  };

  virtual std::vector<Target> get_target() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  virtual std::vector<std::vector<Target>> get_all_targets() {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  // returns a unit definition, and optional additional scale factor and whether
  // histograms should be converted to a pdf (divide out bin width) on
  // finalizing.
  virtual std::tuple<NuHepMC::CrossSection::Units::Unit, double, bool>
  get_cross_section_scaling() const {
    throw IAnalysisUnimplementedInterfaceFunction();
  }

  WeightFunc weight;
  FinalizeFunc finalise;
  LikelihoodFunc likelihood;

  virtual std::string prediction_generation_hint(std::string const &) {
    throw IAnalysisUnimplementedInterfaceFunction();
  };

  virtual ~IAnalysis() {};
};

using AnalysisPtr = std::shared_ptr<IAnalysis>;

} // namespace nuis
