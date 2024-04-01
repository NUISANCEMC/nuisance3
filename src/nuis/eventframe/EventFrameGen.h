#pragma once

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/column_types.h"

#include "nuis/log.h"

#include <functional>
#include <numeric>

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

namespace nuis {

class EventFrameGen : public nuis_named_log("EventFrame") {

public:
  using FilterFunc = std::function<int(HepMC3::GenEvent const &)>;

  template <typename RT>
  using ProjectionFunc = std::function<RT(HepMC3::GenEvent const &)>;
  template <typename RT>
  using ProjectionsFunc =
      std::function<std::vector<RT>(HepMC3::GenEvent const &)>;

  EventFrameGen(INormalizedEventSourcePtr evs, size_t block_size = 500000);
  EventFrameGen filter(FilterFunc filt);

  template <typename RT>
  EventFrameGen add_typed_columns(std::vector<std::string> col_names,
                                  ProjectionsFunc<RT> proj) {

    auto &projs = get_proj_functions<RT>();
    columns.push_back(
        ColumnBlockDefinition{col_names, column_type<RT>::id, projs.size()});
    projs.emplace_back(proj);

    return *this;
  }

  template <typename RT>
  EventFrameGen add_typed_column(std::string col_name,
                                 ProjectionFunc<RT> proj) {
    add_typed_columns<RT>(
        {
            col_name,
        },
        [=](auto const &ev) -> std::vector<RT> {
          return {
              proj(ev),
          };
        });

    return *this;
  }

  EventFrameGen add_columns(std::vector<std::string> col_names,
                            ProjectionsFunc<double> proj) {
    return add_typed_columns<double>(col_names, proj);
  }

  EventFrameGen add_column(std::string col_name, ProjectionFunc<double> proj) {
    return add_typed_column<double>(col_name, proj);
  }

  EventFrameGen limit(size_t nmax);
  EventFrameGen progress(size_t every = 100000);

  EventFrame first();
  EventFrame next();
  EventFrame all();

#ifdef NUIS_ARROW_ENABLED
  std::shared_ptr<arrow::RecordBatch> firstArrow();
  std::shared_ptr<arrow::RecordBatch> nextArrow();
#endif

private:
  INormalizedEventSourcePtr source;

  std::vector<FilterFunc> filters;

  struct ColumnBlockDefinition {
    std::vector<std::string> column_names;
    int typenum;
    size_t proj_index;
  };

  std::vector<ColumnBlockDefinition> columns;

  template <typename RT>
  constexpr std::vector<ProjectionsFunc<RT>> &get_proj_functions() {
    if constexpr (std::is_same_v<RT, bool>) {
      return projectors_bool;
    } else if constexpr (std::is_same_v<RT, int>) {
      return projectors_int;
    } else if constexpr (std::is_same_v<RT, uint>) {
      return projectors_uint64_t;
    } else if constexpr (std::is_same_v<RT, int16_t>) {
      return projectors_int16_t;
    } else if constexpr (std::is_same_v<RT, uint16_t>) {
      return projectors_uint16_t;
    } else if constexpr (std::is_same_v<RT, float>) {
      return projectors_float;
    } else if constexpr (std::is_same_v<RT, double>) {
      return projectors_double;
    }
  }

  template <typename T>
  size_t fill_row_columns(Eigen::ArrayXdRef row, HepMC3::GenEvent const &ev,
                          size_t proj_index, size_t first_col,
                          size_t ncols_to_fill);

  std::vector<ProjectionsFunc<bool>> projectors_bool;
  std::vector<ProjectionsFunc<int>> projectors_int;
  std::vector<ProjectionsFunc<uint>> projectors_uint64_t;
  std::vector<ProjectionsFunc<int16_t>> projectors_int16_t;
  std::vector<ProjectionsFunc<uint16_t>> projectors_uint16_t;
  std::vector<ProjectionsFunc<float>> projectors_float;
  std::vector<ProjectionsFunc<double>> projectors_double;

#ifdef NUIS_ARROW_ENABLED
  template <typename T>
  void fill_array_builder(std::vector<ArrowBuilderPtr> &,
                          HepMC3::GenEvent const &ev, size_t proj_index,
                          size_t first_col, size_t ncols_to_fill);

  arrow::Result<std::shared_ptr<arrow::RecordBatch>> _nextArrow();
#endif

  size_t chunk_size;

  size_t max_events_to_loop;
  size_t progress_report_every;
  size_t nevents;

  // first/next state
  std::vector<std::string> all_column_names;
  size_t n_total_rows;
  size_t neventsprocessed;
  INormalizedEventSource_looper ev_it;
  NormInfo norm_info;
};

} // namespace nuis