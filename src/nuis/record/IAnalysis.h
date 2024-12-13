#pragma once

#include "nuis/eventinput/INormalizedEventSource.h"

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/EventFrameGen.h"

#include "nuis/histframe/BinnedValues.h"

#include "nuis/record/Comparison.h"
#include "nuis/record/FinaliseFunctions.h"
#include "nuis/record/LikelihoodFunctions.h"
#include "nuis/record/WeightFunctions.h"

#include "NuHepMC/UnitsUtils.hxx"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include <filesystem>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace nuis {

using ProjectFunc = std::function<double(HepMC3::GenEvent const &)>;
using SelectFunc = std::function<int(HepMC3::GenEvent const &)>;

// The union of interfaces to different analysis types. Throws if you call the
// wrong function for the wrong analysis, but enables automated comparison and
// full specific analysis interrogation by experts
struct IAnalysis : public std::enable_shared_from_this<IAnalysis> {

  DECLARE_NUISANCE_EXCEPT(IAnalysisUnimplementedInterfaceFunction);

  virtual Comparison process(std::vector<INormalizedEventSourcePtr> &);
  virtual Comparison process(INormalizedEventSourcePtr &);
  virtual Comparison process(EventFrame const &);
#ifdef NUIS_ARROW_ENABLED
  virtual Comparison process(std::shared_ptr<arrow::Table> const &);
  virtual Comparison process(std::shared_ptr<arrow::RecordBatch> const &);
#endif

  virtual void add_to_framegen(EventFrameGen &) const;

  struct Selection {
    std::string fname;
    SelectFunc op;
  };

  // Throws if the actual analysis is more complicated
  virtual Selection get_selection() const;
  virtual std::vector<Selection> get_all_selections() const;

  struct Projection {
    std::string fname;
    ProjectFunc op;
    std::string prettyname;
    std::string units;
  };

  // Throws if the actual analysis is more complicated
  virtual std::vector<Projection> get_projections() const;

  // The outer vector corresponds to the different measurement components, and
  // the inner vector corresponds to the independent variable list for that
  // component. Note this is swapped relative to
  // HEPData::CrossSectionMeasurement
  virtual std::vector<std::vector<Projection>> get_all_projections() const;

  virtual std::vector<BinnedValues> get_data() const;

  struct ProbeFlux {
    int probe_pdg;
    BinnedValues spectrum;
    std::filesystem::path source;
    // dependent variable name if source points to HEPData yaml, or TKey if it
    // points to a root file
    std::string series_name;
  };

  virtual ProbeFlux get_probe_flux(bool = false) const;
  virtual std::vector<ProbeFlux> get_all_probe_fluxes(bool = false) const;

  virtual Eigen::MatrixXd get_covariance_matrix() const;
  virtual Eigen::MatrixXd get_correlation_matrix() const;

  virtual Eigen::MatrixXd get_smearing_matrix() const;
  virtual std::vector<Eigen::MatrixXd> get_all_smearing_matrices() const;

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
    std::string to_str() const;
  };

  virtual std::vector<Target> get_target() const;

  virtual std::vector<std::vector<Target>> get_all_targets() const;

  struct XSScaling {
    NuHepMC::CrossSection::Units::Unit units;
    double extra_scale_factor;
    bool divide_by_bin_width;
  };

  virtual XSScaling get_cross_section_scaling() const;

  virtual std::vector<XSScaling> get_all_cross_section_scalings() const;

  virtual finalise::func get_finalise() const;
  virtual std::vector<finalise::func> get_all_finalise() const;

  likelihood::func likelihood;

  virtual std::string prediction_generation_hint() const;

  virtual ~IAnalysis() {};
};

using AnalysisPtr = std::shared_ptr<IAnalysis>;

} // namespace nuis
