#include "nuis/record/IAnalysis.h"

namespace nuis {

DECLARE_NUISANCE_EXCEPT(IAnalysisUnimplementedInterfaceFunction);

Comparison IAnalysis::process(std::vector<INormalizedEventSourcePtr> &) {
  throw IAnalysisUnimplementedInterfaceFunction();
}
Comparison IAnalysis::process(INormalizedEventSourcePtr &) {
  throw IAnalysisUnimplementedInterfaceFunction();
}
Comparison IAnalysis::process(EventFrame const &) {
  throw IAnalysisUnimplementedInterfaceFunction();
}
#ifdef NUIS_ARROW_ENABLED
Comparison IAnalysis::process(std::shared_ptr<arrow::Table> const &) {
  throw IAnalysisUnimplementedInterfaceFunction();
}
Comparison IAnalysis::process(std::shared_ptr<arrow::RecordBatch> const &) {
  throw IAnalysisUnimplementedInterfaceFunction();
}
#endif

void IAnalysis::add_to_framegen(EventFrameGen &) const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

// Throws if the actual analysis is more complicated
std::pair<std::string, SelectFunc> IAnalysis::get_selection() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}
std::pair<std::vector<std::string>, std::vector<SelectFunc>>
IAnalysis::get_all_selections() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

// Throws if the actual analysis is more complicated
std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
IAnalysis::get_projections() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}
std::pair<std::vector<std::vector<std::string>>,
          std::vector<std::vector<ProjectFunc>>>
IAnalysis::get_all_projections() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<BinnedValues> IAnalysis::get_data() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::pair<int, BinnedValues> IAnalysis::get_probe_flux(bool) const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<std::pair<int, BinnedValues>>
IAnalysis::get_all_probe_fluxes(bool) const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

Eigen::MatrixXd IAnalysis::get_covariance_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

Eigen::MatrixXd IAnalysis::get_correlation_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

Eigen::MatrixXd IAnalysis::get_smearing_matrix() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<IAnalysis::Target> IAnalysis::get_target() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<std::vector<IAnalysis::Target>> IAnalysis::get_all_targets() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::vector<IAnalysis::XSScaling>
IAnalysis::get_all_cross_section_scalings() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

IAnalysis::XSScaling IAnalysis::get_cross_section_scaling() const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

std::string IAnalysis::prediction_generation_hint(std::string const &) const {
  throw IAnalysisUnimplementedInterfaceFunction();
}

} // namespace nuis
