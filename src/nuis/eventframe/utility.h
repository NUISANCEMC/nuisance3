#pragma once

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/column_types.h"

#include "NuHepMC/UnitsUtils.hxx"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include "fmt/core.h"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace nuis {

template <typename EFT>
double get_best_fatx_per_sumw_estimate(
    EFT const &ef, NuHepMC::CrossSection::Units::Unit const &xs_units) {

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
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    if (!ef->num_rows()) {
      throw std::runtime_error(
          "in SingleDistributionAnalysis::process was passed an arrow "
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
      throw std::runtime_error(
          "in SingleDistributionAnalysis::process was passed an "
          "arrow table with no rows");
    }

    return best_estimate;
  }
#endif
}

namespace detail::eft {

template <typename EFT, typename COLTYPE>
inline bool is_valid_col(COLTYPE const &col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return col != EventFrame::npos;
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return bool(col);
  }
#endif
}

template <typename EFT> inline int num_rows(EFT const &ef) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return ef.table.rows();
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return ef->num_rows();
  }
#endif
}

template <typename EFT, typename ROWTYPE, typename COLTYPE>
inline auto get_entry(EFT const &ef, ROWTYPE const &row, COLTYPE const &col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return ef.table(row, col);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return col(row);
  }
#endif
}

template <typename CAST_TO, typename EFT>
inline auto require_column_index(EFT const &ef, std::string const &cn) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return ef.require_column_index(cn);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return get_col_cast_to<CAST_TO>(ef, cn);
  }
#endif
}

} // namespace detail::eft

} // namespace nuis
