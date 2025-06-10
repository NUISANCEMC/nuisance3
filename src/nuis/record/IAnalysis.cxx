#include "nuis/record/IAnalysis.h"

#include "fmt/core.h"

namespace nuis {

DECLARE_NUISANCE_EXCEPT(IAnalysisUnimplementedInterfaceFunction);

Comparison IAnalysis::process(std::vector<NormalizedEventSourcePtr> &) {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Comparison IAnalysis::process(std::vector<NormalizedEventSourcePtr> "
         "&)";
}
Comparison IAnalysis::process(NormalizedEventSourcePtr &) {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Comparison IAnalysis::process(NormalizedEventSourcePtr &)";
}
Comparison IAnalysis::process(EventFrame const &) {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Comparison IAnalysis::process(EventFrame const &)";
}
#ifdef NUIS_ARROW_ENABLED
Comparison IAnalysis::process(std::shared_ptr<arrow::Table> const &) {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Comparison IAnalysis::process(std::shared_ptr<arrow::Table> const &)";
}
Comparison IAnalysis::process(std::shared_ptr<arrow::RecordBatch> const &) {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Comparison IAnalysis::process(std::shared_ptr<arrow::RecordBatch> "
         "const &)";
}
#endif

void IAnalysis::add_to_framegen(EventFrameGen &) const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "void IAnalysis::add_to_framegen(EventFrameGen &) const";
}

// Throws if the actual analysis is more complicated
IAnalysis::Selection IAnalysis::get_selection() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "IAnalysis::Selectio IAnalysis::get_selection() const ";
}
std::vector<IAnalysis::Selection> IAnalysis::get_all_selections() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<IAnalysis::Selection> "
         "IAnalysis::get_all_selections() const";
}

// Throws if the actual analysis is more complicated
std::vector<IAnalysis::Projection> IAnalysis::get_projections() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<IAnalysis::Projection> "
         "IAnalysis::get_projections() const";
}
std::vector<std::vector<IAnalysis::Projection>>
IAnalysis::get_all_projections() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<std::vector<IAnalysis::Projection>> "
         "IAnalysis::get_all_projections() const";
}

std::vector<BinnedValues> IAnalysis::get_data() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<BinnedValues> IAnalysis::get_data() const";
}

IAnalysis::ProbeFlux IAnalysis::get_probe_flux(bool) const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "IAnalysis::ProbeFlux IAnalysis::get_probe_flux(bool) const";
}

std::vector<IAnalysis::ProbeFlux> IAnalysis::get_all_probe_fluxes(bool) const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<IAnalysis::ProbeFlux> "
         "IAnalysis::get_all_probe_fluxes(bool) const";
}

Eigen::MatrixXd IAnalysis::get_covariance_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Eigen::MatrixXd IAnalysis::get_covariance_matrix() const";
}

Eigen::MatrixXd IAnalysis::get_correlation_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Eigen::MatrixXd IAnalysis::get_correlation_matrix() const";
}

Eigen::MatrixXd IAnalysis::get_smearing_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "Eigen::MatrixXd IAnalysis::get_smearing_matrix() const";
}

std::vector<Eigen::MatrixXd> IAnalysis::get_all_smearing_matrices() const {
  throw IAnalysisUnimplementedInterfaceFunction()
      << "std::vector<Eigen::MatrixXd> IAnalysis::get_all_smearing_matrices() "
         "const";
}

std::vector<IAnalysis::Target> IAnalysis::get_target() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<std::vector<IAnalysis::Target>> IAnalysis::get_all_targets() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

IAnalysis::XSScaling IAnalysis::get_cross_section_scaling() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<IAnalysis::XSScaling>
IAnalysis::get_all_cross_section_scalings() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

finalise::func IAnalysis::get_finalise() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}
std::vector<finalise::func> IAnalysis::get_all_finalise() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::string IAnalysis::prediction_generation_hint() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::string IAnalysis::Target::to_str() const {
  // ±10LZZZAAAI
  return fmt::format("100{:03}{:03}0", Z, A) +
         (weight_by_mass == 1 ? "" : fmt::format("[{}]", weight_by_mass));
}

} // namespace nuis
