#pragma once

#include "nuis/record/IAnalysis.h"

#include "nuis/eventframe/utility.h"

#include "nuis/histframe/frame_fill.h"
#include "nuis/histframe/utility.h"

#include "spdlog/fmt/bundled/core.h"

namespace nuis {

// An analysis that describes a single distribution of arbitrary dimension, may
// include smearing, an error matrix, and a composite targets, but only a single
// probe spectrum and selection function.
struct SingleDistributionAnalysis : public IAnalysis {

  BinnedValues data;
  HistFrame prediction;

  std::string select_name;
  SelectFunc select;

  std::vector<std::string> projection_names;
  std::vector<ProjectFunc> projections;

  std::vector<IAnalysis::Target> target;

  IAnalysis::ProbeFlux probe_count;

  XSScaling xsscale;

  Eigen::MatrixXd error;
  Eigen::MatrixXd smearing;

  finalise::func finalise;

  template <typename EFT> Comparison process_batched(EFT const &batch) {
    // note that this doesn't reset the prediction so can be used for batched
    // processing
    fill(prediction, batch, projection_names, fill_column(0),
         fill_if(select_name));

    // acting on a copy of the current prediction also enables batched
    // processing
    HistFrame comparison_prediction = prediction;

    if (smearing.size()) { // probably want this to be a hook really
      for (int icol = 0; icol < comparison_prediction.sumweights.cols();
           ++icol) {
        comparison_prediction.sumweights.col(icol) =
            smearing * comparison_prediction.sumweights.col(icol).matrix();
        comparison_prediction.variances.col(icol) =
            smearing * comparison_prediction.variances.col(icol).matrix();
      }
    }

    auto final_prediction =
        finalise(comparison_prediction,
                 get_best_fatx_per_sumw_estimate(batch, xsscale.units) *
                     xsscale.extra_scale_factor);

    Comparison comp{{
                        data,
                    },
                    {
                        final_prediction,
                    },
                    nullptr};

    // take an explicit copy of the closure so that the comparison likelihood
    // function doesn't depend on the lifetime of this object.
    auto ana_lhood_func = this->likelihood;

    // holds two copies of the data, but I don't think it should be a huge
    // problem
    comp.likelihood = [=]() { return ana_lhood_func(comp); };

    return comp;
  }

  Comparison process(EventFrame const &ef) {
    prediction.reset();
    return process_batched(ef);
  }

#ifdef NUIS_ARROW_ENABLED
  Comparison process(std::shared_ptr<arrow::Table> const &at) {
    prediction.reset();
    return process_batched(at);
  }
  Comparison process(std::shared_ptr<arrow::RecordBatch> const &rb) {
    prediction.reset();
    return process_batched(rb);
  }
#endif

  Comparison process(INormalizedEventSourcePtr &events) {

    auto efg = EventFrameGen(events).filter(select);
    add_to_framegen(efg);

    prediction.reset();

    Comparison comp;

    auto batch = efg.first(1E4);
    while (batch) {
      comp = process_batched(batch);
      batch = efg.next(1E4);
    }

    return comp;
  }

  void add_to_framegen(EventFrameGen &efg) const {

    auto sel_col_check = efg.has_typed_column<int>(select_name);
    if (!sel_col_check.first) { // doesn't have a column with that name
      efg.add_typed_column<int>(select_name, select);
    } else if (!sel_col_check.second) {
      throw std::runtime_error(fmt::format(
          "when adding column named {} to EventFrameGen, a column with the "
          "same name but a different type already exists.",
          select_name));
    }

    for (size_t pi = 0; pi < projections.size(); ++pi) {
      if (!efg.has_column(projection_names[pi])) {
        efg.add_column(projection_names[pi], projections[pi]);
      }
    }
  }

  std::pair<std::string, SelectFunc> get_selection() const {
    return {select_name, select};
  }

  std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() const {
    return {projection_names, projections};
  }

  std::vector<BinnedValues> get_data() const {
    return {
        data,
    };
  }

  IAnalysis::ProbeFlux get_probe_flux(bool count_density = false) const {
    if (count_density) {
      return {probe_count.probe_pdg, ToCountDensity(probe_count.spectrum),
              probe_count.source, probe_count.series_name};
    }
    return probe_count;
  }

  std::vector<IAnalysis::Target> get_target() const { return target; }

  Eigen::MatrixXd get_covariance_matrix() const {
    if (!error.size()) {
      throw std::runtime_error("analysis has no covariance matrix set.");
    }
    return error;
  }

  Eigen::MatrixXd get_smearing_matrix() const {
    if (!smearing.size()) {
      throw std::runtime_error("analysis has no smearing matrix set.");
    }
    return smearing;
  }

  IAnalysis::XSScaling get_cross_section_scaling() const { return xsscale; }

  finalise::func get_finalise() const { return finalise; }

  std::string prediction_generation_hint() const {
    std::stringstream ss;
    for (size_t tgti = 0; tgti < target.size(); ++tgti) {
      ss << target[tgti].to_str() << ((tgti + 1) == target.size() ? "" : ",");
    }
    return fmt::format("-P {} -f {},{} -t {}", probe_count.probe_pdg,
                       probe_count.source.native(), probe_count.series_name,
                       ss.str());
  }
};

} // namespace nuis
