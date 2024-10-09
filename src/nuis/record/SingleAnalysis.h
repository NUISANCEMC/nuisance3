#pragma once

#include "nuis/record/IAnalysis.h"

#include "nuis/histframe/fill_from_EventFrame.h"
#include "nuis/histframe/utility.h"

#include "NuHepMC/UnitsUtils.hxx"

#include "fmt/core.h"

namespace nuis {

struct SingleAnalysis : public IAnalysis {

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

  template <typename EFT>
  double get_best_fatx_per_sumw_estimate(EFT const &ef) const {

    std::string fatx_colname;

    if (xs_units.tgtscale ==
        NuHepMC::CrossSection::Units::TargetScale::PerTarget) {
      fatx_colname = "fatx_per_sumw.pb_per_target.estimate";
    } else if (xs_units.tgtscale ==
               NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon) {
      fatx_colname = "fatx_per_sumw.pb_per_nucleon.estimate";
    } else {
      std::stringstream ss;
      ss << xs_units.tgtscale;
      throw std::runtime_error(
          fmt::format("When retrieving best fatx_per_sumw estimate from "
                      "proferred event frame, the analysis target "
                      "scale was: {}, which is invalid for automatic scaling.",
                      ss.str()));
    }

    static std::map<NuHepMC::CrossSection::Units::Scale, double> const
        xsunit_factors = {
            {NuHepMC::CrossSection::Units::Scale::pb,
             NuHepMC::CrossSection::Units::pb},
            {NuHepMC::CrossSection::Units::Scale::cm2,
             NuHepMC::CrossSection::Units::cm2},
            {NuHepMC::CrossSection::Units::Scale::cm2_ten38,
             NuHepMC::CrossSection::Units::cm2_ten38},
        };

    double units_scale =
        NuHepMC::CrossSection::Units::pb / xsunit_factors.at(xs_units.scale);

    if constexpr (std::is_same_v<EFT, EventFrame>) {
      return ef.col(fatx_colname).tail(1)(0) * units_scale;
    }
#ifdef NUIS_ARROW_ENABLED
    else if constexpr (std::is_same_v<EFT,
                                      std::shared_ptr<arrow::RecordBatch>>) {
      if (!ef->num_rows()) {
        throw std::runtime_error(
            "in SingleAnalysis::process was passed an arrow "
            "record batch with no rows.");
      }
      return get_col_cast_to<double>(ef, fatx_colname)(ef->num_rows() - 1) *
             units_scale;
    } else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::Table>>) {

      double best_estimate = 0;
      for (auto rb : arrow::TableBatchReader(ef)) {
        if (!rb.ValueOrDie()->num_rows()) {
          break;
        }
        best_estimate = get_col_cast_to<double>(rb.ValueOrDie(), fatx_colname)(
                            rb.ValueOrDie()->num_rows() - 1) *
                        units_scale;
      }

      if (best_estimate == 0) {
        throw std::runtime_error("in SingleAnalysis::process was passed an "
                                 "arrow table with no rows");
      }

      return best_estimate;
    }
#endif
  }

  template <typename EFT>
  Comparison process_batched(EFT const &batch, double fatx_per_sumweights) {
    // note that this doesn't reset the prediction so can be used for batched
    // processing

    if constexpr (std::is_same_v<EFT, EventFrame>) {
      fill_from_EventFrame_if(prediction, batch, select_name, projection_names);
    }
#ifdef NUIS_ARROW_ENABLED
    else {
      fill_from_Arrow_if(prediction, batch, select_name, projection_names);
    }
#endif

    // acting on a copy of the current prediction also enables batched
    // processing
    HistFrame comparison_prediction = prediction;

    if (smearing.size()) { // probably want this to be a hook really
      for (int icol = 0; icol < comparison_prediction.sumweights.cols();
           ++icol) {
        comparison_prediction.sumweights.col(icol) =
            smearing * comparison_prediction.sumweights.col(icol).matrix();
        comparison_prediction.variances.col(icol) =
            (smearing * smearing) *
            comparison_prediction.variances.col(icol).matrix();
      }
    }

    auto final_prediction =
        finalise(comparison_prediction, fatx_per_sumweights * extra_unit_scale);

    Comparison comp{{
                        data,
                    },
                    {
                        final_prediction,
                    },
                    nullptr};
    // holds two copies of the data, but I don't think it should be a huge
    // problem
    comp.likelihood = [=]() { return likelihood(comp); };

    return comp;
  }

  Comparison process(EventFrame const &ef) {
    prediction.reset();
    return process_batched(ef, get_best_fatx_per_sumw_estimate(ef));
  }

#ifdef NUIS_ARROW_ENABLED
  Comparison process(std::shared_ptr<arrow::Table> const &at) {
    prediction.reset();
    return process_batched(at, get_best_fatx_per_sumw_estimate(at));
  }
  Comparison process(std::shared_ptr<arrow::RecordBatch> const &rb) {
    prediction.reset();
    return process_batched(rb, get_best_fatx_per_sumw_estimate(rb));
  }
#endif

  Comparison process(INormalizedEventSourcePtr &events) {

    auto efg = EventFrameGen(events).filter(select);
    add_to_framegen(efg);

    prediction.reset();

    Comparison comp;

    auto batch = efg.first(1E4);
    while (batch) {
      comp = process_batched(batch,
                             events->norm_info(xs_units).fatx_per_sumweights());
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

  std::pair<int, BinnedValues>
  get_probe_flux(bool count_density = false) const {
    if (count_density) {
      return {probe_particle, ToCountDensity(probe_flux_count)};
    }
    return {probe_particle, probe_flux_count};
  }

  std::vector<IAnalysis::Target> get_target() const { return target; }

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

  IAnalysis::XSScaling get_cross_section_scaling() const {
    return {xs_units, extra_unit_scale, per_bin_width};
  }

  std::string
  prediction_generation_hint(std::string const &generator_name) const {
    std::stringstream ss("");
    if (generator_name == "NEUT") {
      // ss << "nuis gen NEUT -P " << probe_particle << " -t " << target << " -f
      // ";
    }
    return ss.str();
  }
};

} // namespace nuis
