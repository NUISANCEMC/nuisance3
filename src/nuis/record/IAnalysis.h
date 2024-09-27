#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/eventframe/EventFrame.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/record/Comparison.h"
#include "nuis/record/FinaliseFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "NuHepMC/UnitsUtils.hxx"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace nuis {

using ProjectFunc = std::function<double(HepMC3::GenEvent const &)>;
using SelectFunc = std::function<int(HepMC3::GenEvent const &)>;

struct IAnalysis {

  NEW_NUISANCE_EXCEPT(IAnalysisUnimplementedInterfaceFunction);

  virtual Comparison process(std::vector<INormalizedEventSourcePtr> &);
  virtual Comparison process(INormalizedEventSourcePtr &);
  virtual Comparison process(EventFrame const &);
#ifdef NUIS_ARROW_ENABLED
  virtual Comparison process(std::shared_ptr<arrow::Table> const &);
  virtual Comparison process(std::shared_ptr<arrow::RecordBatch> const &);
#endif

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::string, SelectFunc> get_selection() const;
  virtual std::pair<std::vector<std::string>, std::vector<SelectFunc>>
  get_all_selections() const;

  // Throws if the actual analysis is more complicated
  virtual std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() const;
  virtual std::pair<std::vector<std::vector<std::string>>,
                    std::vector<std::vector<ProjectFunc>>>
  get_all_projections() const;

  virtual std::vector<BinnedValues> get_data() const;

  virtual std::pair<int, BinnedValues> get_probe_flux(bool = false) const;
  virtual std::vector<std::pair<int, BinnedValues>>
  get_all_probe_fluxes(bool = false) const;

  virtual Eigen::MatrixXd get_covariance_matrix() const;
  virtual Eigen::MatrixXd get_correlation_matrix() const;

  virtual Eigen::MatrixXd get_smearing_matrix() const;

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

  virtual std::vector<Target> get_target() const;

  virtual std::vector<std::vector<Target>> get_all_targets() const;

  struct XSScaling {
    NuHepMC::CrossSection::Units::Unit units;
    double extra_scale_factor;
    bool divide_by_bin_width;
  };

  virtual std::vector<XSScaling> get_all_cross_section_scalings() const;

  virtual XSScaling get_cross_section_scaling() const;

  weight::func weight;
  finalise::func finalise;
  likelihood::func likelihood;

  virtual std::string prediction_generation_hint(std::string const &) const;

  virtual ~IAnalysis() {};
};

using AnalysisPtr = std::shared_ptr<IAnalysis>;

} // namespace nuis
