#pragma once

#include "nuis/frame/Frame.h"
#include "nuis/frame/column_types.h"

#include "nuis/log.h"

#include <functional>
#include <numeric>

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

namespace nuis {

class FrameGen : public nuis_named_log("Frame") {

public:
  using FilterFunc = std::function<int(HepMC3::GenEvent const &)>;

  template <typename RT = double>
  using ProjectionFunc = std::function<RT(HepMC3::GenEvent const &)>;
  template <typename RT = double>
  using ProjectionsFunc =
      std::function<std::vector<RT>(HepMC3::GenEvent const &)>;

  // PS An option to input a vector of functions is also needed (instead of
  // requiring a lambda to build it) LP Is add_column("name",
  // func).add_column("name", func).add_column("name", func) not okay?

  FrameGen(INormalizedEventSourcePtr evs, size_t block_size = 500000);
  FrameGen filter(FilterFunc filt);

  template <typename RT = double>
  FrameGen add_typed_columns(std::vector<std::string> col_names,
                             ProjectionsFunc<RT> proj) {

    auto &projs = get_proj_functions<RT>();
    columns.push_back(
        ColumnBlockDefinition{col_names, column_type<RT>::id, projs.size()});
    projs.emplace_back(proj);

    return *this;
  }

  template <typename RT = double>
  FrameGen add_typed_column(std::string col_name, ProjectionFunc<RT> proj) {
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

  FrameGen add_columns(std::vector<std::string> col_names,
                       ProjectionsFunc<double> proj) {
    return add_typed_columns<double>(col_names, proj);
  }

  FrameGen add_column(std::string col_name, ProjectionFunc<double> proj) {
    return add_typed_column<double>(col_name, proj);
  }

  FrameGen limit(size_t nmax);
  FrameGen progress(size_t every = 100000);

  Frame first();
  Frame next();
  Frame all();

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

  template <typename RT = double>
  constexpr std::vector<ProjectionsFunc<RT>> &get_proj_functions() {
    if constexpr (std::is_same_v<RT, int>) {
      return projectors_int;
    } else if (std::is_same_v<RT, double>) {
      return projectors_double;
    }
  }
  template <typename T>
  size_t fill_row_columns(Eigen::ArrayXdRef row, HepMC3::GenEvent const &ev,
                          size_t proj_index, size_t first_col);

  std::vector<ProjectionsFunc<int>> projectors_int;
  std::vector<ProjectionsFunc<double>> projectors_double;

#ifdef NUIS_ARROW_ENABLED
  template <typename T>
  void fill_array_builder(std::vector<std::unique_ptr<arrow::ArrayBuilder>> &,
                          HepMC3::GenEvent const &ev, size_t proj_index,
                          size_t first_col, size_t ncols_to_fill);

  arrow::Result<std::shared_ptr<arrow::RecordBatch>> _nextArrow();
#endif

  size_t chunk_size;

  size_t max_events_to_loop;
  size_t progress_report_every;
  size_t nevents;

  size_t GetNCols();

  // first/next state
  std::vector<std::string> all_column_names;
  size_t n_total_rows;
  size_t neventsprocessed;
  INormalizedEventSource_looper ev_it;
  NormInfo norm_info;
};

} // namespace nuis