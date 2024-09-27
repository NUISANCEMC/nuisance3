#pragma once

#include "nuis/record/IAnalysis.h"

#include "nuis/histframe/utility.h"

#include "NuHepMC/UnitsUtils.hxx"

namespace nuis {

struct SimpleAnalysis : public IAnalysis {

  BinnedValues data;
  HistFrame prediction;

  std::string select_name;
  SelectFunc select;

  std::vector<std::string> projection_names;
  std::vector<ProjectFunc> projections;

  std::vector<IAnalysis::Target> target;

  int probe_particle;
  BinnedValues probe_flux_count;

  NuHepMC::CrossSection::Units::Unit xs_units;
  // required because NuHepMC doesn't consider /NProtons or /NNeutrons, so we
  // have to include this
  double extra_unit_scale;
  bool per_bin_width;

  Eigen::MatrixXd error;
  Eigen::MatrixXd smearing;

  Comparison process(INormalizedEventSourcePtr &events) {

    prediction.reset();

    static std::vector<double> proj_values;

    for (auto const &[evt, cvw] : events) {
      if (!select(*evt)) {
        continue;
      }
      proj_values.clear();
      for (auto const &proj : projections) {
        proj_values.push_back(proj(*evt));
      }
      prediction.fill(proj_values, cvw);
    }

    if (smearing.size()) { // probably want this to be a hook really
      for (int icol = 0; icol < prediction.sumweights.cols(); ++icol) {
        prediction.sumweights.col(icol) =
            smearing * prediction.sumweights.col(icol).matrix();
        prediction.variances.col(icol) =
            (smearing * smearing) *
            prediction.variances.col(icol).matrix();
      }
    }

    auto final_prediction =
        finalise(prediction, events->norm_info(xs_units).fatx_per_sumweights() *
                                 extra_unit_scale);

    return Comparison{{
                          data,
                      },
                      {
                          final_prediction,
                      },
                      [=]() -> double {
                        auto const &data_rate = data.get_bin_contents().col(0);
                        auto const &pred_rate =
                            final_prediction.get_bin_contents().col(0);
                        auto const &diffvect = (data_rate - pred_rate).matrix();
                        return diffvect.transpose() * error * diffvect;
                      }};
  }

  std::pair<std::string, SelectFunc> get_selection() {
    return {select_name, select};
  }

  std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() {
    return {projection_names, projections};
  }

  std::vector<BinnedValues> get_data() {
    return {
        data,
    };
  }

  std::pair<int, BinnedValues> get_probe_flux(bool count_density = false) {
    if (count_density) {
      return {probe_particle, ToCountDensity(probe_flux_count)};
    }
    return {probe_particle, probe_flux_count};
  }

  std::vector<IAnalysis::Target> get_target() { return target; }

  Eigen::MatrixXd get_covariance_matrix() const {
    if (!error.size()) {
      throw std::runtime_error("analysis has no covaraince matrix set.");
    }
    return error;
  }

  Eigen::MatrixXd get_smearing_matrix() const {
    if (!smearing.size()) {
      throw std::runtime_error("analysis has no smearing matrix set.");
    }
    return smearing;
  }

  std::tuple<NuHepMC::CrossSection::Units::Unit, double, bool>
  get_cross_section_scaling() const {
    return {xs_units, extra_unit_scale, per_bin_width};
  }

  std::string prediction_generation_hint(std::string const &generator_name) {
    std::stringstream ss("");
    if (generator_name == "NEUT") {
      // ss << "nuis gen NEUT -P " << probe_particle << " -t " << target << " -f
      // ";
    }
    return ss.str();
  }
};

} // namespace nuis
