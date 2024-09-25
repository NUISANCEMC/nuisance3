#pragma once

#include "nuis/record/IAnalysis.h"

#include "NuHepMC/UnitsUtils.hxx"

namespace nuis {

struct SimpleAnalysis : public IAnalysis {

  BinnedValues data;
  HistFrame prediction;

  std::string select_name;
  SelectFunc select;

  std::vector<std::string> projection_names;
  std::vector<ProjectFunc> projections;

  int probe_particle;
  BinnedValues probe_flux;

  std::string target;

  NuHepMC::CrossSection::Units::Unit xs_units;
  // required because NuHepMC doesn't consider /NProtons or /NNeutrons, so we
  // have to include this
  double extra_unit_scale;

  Eigen::MatrixXd error;

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

  std::string prediction_generation_hint(std::string const &generator_name) {
    std::stringstream ss("");
    if (generator_name == "NEUT") {
      ss << "nuis gen NEUT -P " << probe_particle << " -t " << target << " -f ";
    }
    return ss.str();
  }
};

} // namespace nuis
