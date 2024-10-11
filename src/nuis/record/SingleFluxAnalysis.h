#pragma once

#include "nuis/record/IAnalysis.h"

#include "nuis/eventframe/utility.h"

#include "nuis/histframe/frame_fill.h"
#include "nuis/histframe/utility.h"

#include "NuHepMC/UnitsUtils.hxx"

#include "spdlog/fmt/bundled/core.h"

namespace nuis {

// An analysis that describes multiple distributions of arbitrary dimension,
// each may include smearing, there may be an error matrix across all
// distributions, each may have different, possibly composite, targets, and each
// may have a separate selection function but only a single probe spectrum.
struct SingleFluxAnalysis : public IAnalysis {

  std::vector<BinnedValues> data;
  std::vector<HistFrame> predictions;

  std::vector<std::string> select_names;
  std::vector<SelectFunc> selects;

  std::vector<std::vector<std::string>> projection_names;
  std::vector<std::vector<ProjectFunc>> projections;

  std::vector<std::vector<IAnalysis::Target>> targets;

  IAnalysis::ProbeFlux probe_count;

  std::vector<XSScaling> xsscales;

  Eigen::MatrixXd error;
  std::vector<Eigen::MatrixXd> smearings;

  template <typename EFT>
  BinnedValues process_dist_batched(EFT const &batch, size_t dist_i) {

    std::string select_name;

    if (select_names.size() == 1) {
      select_name = select_names[0];
    } else if (select_names.size() == data.size()) {
      select_name = select_names[dist_i];
    } else {
      throw std::runtime_error(
          fmt::format("when building distribution {}, found that the dataset "
                      "had {} selection function names ({}), but as we have {} "
                      "distributions, the mapping is ambiguous.",
                      dist_i, select_names.size(), select_names, data.size()));
    }

    std::vector<std::string> dist_projection_names;

    if (projection_names.size() ==
        1) { // use the same projections for each distribution
      dist_projection_names = projection_names[0];
    } else if (projection_names.size() ==
               data.size()) { // use a different projection for each
                              // distribution
      dist_projection_names = projection_names[dist_i];
    } else {
      throw std::runtime_error(
          fmt::format("when building distribution {}, found that the dataset "
                      "had {} sets of projection function names for this, but "
                      "as we have {} distributions, the mapping is ambiguous.",
                      dist_i, projection_names.size(), data.size()));
    }

    // note that this doesn't reset the prediction so can be used for
    // batched processing
    fill(predictions[dist_i], batch, dist_projection_names, fill_column(0),
         fill_if(select_name));

    // acting on a copy of the current prediction also enables batched
    // processing
    HistFrame comparison_prediction = predictions[dist_i];

    if (smearings.size()) { // probably want this to be a hook really

      if (smearings.size() != data.size()) {
        throw std::runtime_error(fmt::format(
            "when building distribution {}, found that the dataset had {} "
            "smearing matrices, but we have {} distributions.",
            dist_i, smearings.size(), data.size()));
      }

      // some sub measurements might not be smeared, in which case they will be
      // empty matrices
      if (smearings[dist_i].rows()) {
        for (int icol = 0; icol < comparison_prediction.sumweights.cols();
             ++icol) {
          comparison_prediction.sumweights.col(icol) =
              smearings[dist_i] *
              comparison_prediction.sumweights.col(icol).matrix();
          comparison_prediction.variances.col(icol) =
              smearings[dist_i] *
              comparison_prediction.variances.col(icol).matrix();
        }
      }
    }

    XSScaling xsscale;

    if (xsscales.size() == 1) {
      xsscale = xsscales[0];
    } else if (xsscales.size() == data.size()) {
      xsscale = xsscales[dist_i];
    } else {
      throw std::runtime_error(
          fmt::format("when building distribution {}, found that the dataset "
                      "had {} cross section scalings, but as we have {} "
                      "distributions, the mapping is ambiguous.",
                      dist_i, xsscales.size(), data.size()));
    }

    return finalise_all[dist_i](
        comparison_prediction,
        get_best_fatx_per_sumw_estimate(batch, xsscale.units) *
            xsscale.extra_scale_factor);
  }

  template <typename EF> Comparison process_batched(EF const &ef) {
    std::vector<BinnedValues> finalised_predictions;
    for (size_t dist_i = 0; dist_i < data.size(); ++dist_i) {
      finalised_predictions.push_back(process_dist_batched(ef, dist_i));
    }
    Comparison comp{data, finalised_predictions, nullptr};
    comp.likelihood = [=]() { return likelihood(comp); };
    return comp;
  }

  Comparison process(EventFrame const &ef) {
    for (auto &p : predictions) {
      p.reset();
    }
    return process_batched(ef);
  }

#ifdef NUIS_ARROW_ENABLED
  Comparison process(std::shared_ptr<arrow::Table> const &at) {
    for (auto &p : predictions) {
      p.reset();
    }
    return process_batched(ef);
  }
  Comparison process(std::shared_ptr<arrow::RecordBatch> const &rb) {
    for (auto &p : predictions) {
      p.reset();
    }
    return process_batched(ef);
  }
#endif

  Comparison process(INormalizedEventSourcePtr &events) {

    auto efg = EventFrameGen(events);
    add_to_framegen(efg);

    for (auto &p : predictions) {
      p.reset();
    }

    Comparison comp;

    auto batch = efg.first(1E4);
    while (batch) {
      comp = process_batched(batch);
      batch = efg.next(1E4);
    }

    return comp;
  }

  Comparison process(std::vector<INormalizedEventSourcePtr> &event_vectors) {

    if (event_vectors.size() != data.size()) {
      throw std::runtime_error(
          fmt::format("when processing event vectors, found that the dataset "
                      "had {} components, but we were passed {} event vectors, "
                      "the mapping is ambiguous.",
                      data.size(), event_vectors.size()));
    }

    Comparison comp;
    comp.data = data;
    comp.predictions.resize(data.size());
    for (size_t dist_i = 0; dist_i < data.size(); ++dist_i) {
      auto &events = event_vectors[dist_i];

      auto efg = EventFrameGen(events);

      if (select_names.size() == 1) {
        efg.filter(selects[0]);
      } else if (select_names.size() == data.size()) {
        efg.filter(selects[dist_i]);
      } else {
        throw std::runtime_error(
            fmt::format("when building distribution {}, found that the dataset "
                        "had {} selection functions, but as we have {} "
                        "distributions, the mapping is ambiguous.",
                        dist_i, selects.size(), data.size()));
      }

      add_to_framegen(efg);

      predictions[dist_i].reset();

      auto batch = efg.first(1E4);
      while (batch) {
        comp.predictions[dist_i] = process_dist_batched(batch, dist_i);
        batch = efg.next(1E4);
      }
    }
    comp.likelihood = [=]() { return likelihood(comp); };

    return comp;
  }

  void add_to_framegen(EventFrameGen &efg) const {

    for (size_t si = 0; si < select_names.size(); ++si) {
      auto sel_col_check = efg.has_typed_column<int>(select_names[si]);
      if (!sel_col_check.first) { // doesn't have a column with that name
        efg.add_typed_column<int>(select_names[si], selects[si]);
      } else if (!sel_col_check.second) {
        throw std::runtime_error(fmt::format(
            "when adding column named {} to EventFrameGen, a column with the "
            "same name but a different type already exists.",
            select_names[si]));
      }
    }

    for (size_t var_i = 0; var_i < projections.size(); ++var_i) {
      for (size_t pi = 0; pi < projections[var_i].size(); ++pi) {
        if (!efg.has_column(projection_names[var_i][pi])) {
          efg.add_column(projection_names[var_i][pi], projections[var_i][pi]);
        }
      }
    }
  }

  std::pair<std::string, SelectFunc> get_selection() const {
    if (select_names.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_selection can only be used for measurements "
          "with a single selection specifier, but we find: {} select_names.",
          select_names.size()));
    }

    if (selects.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_selection can only be used for measurements "
          "with a single selection specifier, but we find: {} selects.",
          selects.size()));
    }

    return {select_names[0], selects[0]};
  }
  std::pair<std::vector<std::string>, std::vector<SelectFunc>>
  get_all_selections() const {
    if (!select_names.size()) {
      throw std::runtime_error("analysis had no selection names set.");
    }

    if (!selects.size()) {
      throw std::runtime_error("analysis had no selection functions set.");
    }

    return {select_names, selects};
  }

  std::pair<std::vector<std::string>, std::vector<ProjectFunc>>
  get_projections() const {
    if (projection_names.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_projections can only be used for measurements "
          "with a single set of projections, but we find: {} sets of "
          "projection_names.",
          projection_names.size()));
    }

    if (projections.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_projections can only be used for measurements "
          "with a single set of projections, but we find: {} sets of "
          "projections.",
          projections.size()));
    }

    return {projection_names[0], projections[0]};
  }
  std::pair<std::vector<std::vector<std::string>>,
            std::vector<std::vector<ProjectFunc>>>
  get_all_projections() const {
    if (!projection_names.size()) {
      throw std::runtime_error("analysis had no projection names set.");
    }

    if (!projections.size()) {
      throw std::runtime_error("analysis had no projection functions set.");
    }

    return {projection_names, projections};
  }

  std::vector<BinnedValues> get_data() const { return data; }

  IAnalysis::ProbeFlux get_probe_flux(bool count_density = false) const {
    if (count_density) {
      return {probe_count.probe_pdg, ToCountDensity(probe_count.spectrum),
              probe_count.source};
    }
    return {probe_count.probe_pdg, probe_count.spectrum, probe_count.source};
  }

  std::vector<IAnalysis::Target> get_target() const {
    if (targets.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_target can only be used for measurements "
          "with a single target specifier, but we find: {} targets.",
          targets.size()));
    }
    return targets[0];
  }
  std::vector<std::vector<Target>> get_all_targets() const {
    if (!targets.size()) {
      throw std::runtime_error("analysis had no target specification sets.");
    }
    return targets;
  }

  Eigen::MatrixXd get_covariance_matrix() const {
    if (!error.size()) {
      throw std::runtime_error("analysis has no covariance matrix set.");
    }
    return error;
  }

  Eigen::MatrixXd get_smearing_matrix() const {
    if (smearings.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_smearing_matrix can only be used for measurements "
          "with a single smearing matrix, but we find: {} smearing "
          "matricies.",
          smearings.size()));
    }
    return smearings[0];
  }

  std::vector<Eigen::MatrixXd> get_all_smearing_matrices() const {
    if (!smearings.size()) {
      throw std::runtime_error("analysis has no smearing matrices set.");
    }
    return smearings;
  }

  IAnalysis::XSScaling get_cross_section_scaling() const {
    if (xsscales.size() != 1) {
      throw std::runtime_error(fmt::format(
          "IAnalysis::get_cross_section_scaling can only be used for "
          "measurements with a single cross section scaling, but we find: {} "
          "scalings.",
          xsscales.size()));
    }
    return xsscales[0];
  }
  std::vector<XSScaling> get_all_cross_section_scalings() const {
    if (!xsscales.size()) {
      throw std::runtime_error("analysis has no cross section scalings.");
    }
    return xsscales;
  }

  std::string
  prediction_generation_hint(std::string const &generator_name) const {
    std::stringstream ss("");
    if (generator_name == "NEUT") {
      // ss << "nuis gen NEUT -P " << probe_particle << " -t " << target << "
      // -f
      // ";
    }
    return ss.str();
  }
};

} // namespace nuis
