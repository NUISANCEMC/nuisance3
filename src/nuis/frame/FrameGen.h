#pragma once

#include "nuis/frame/Frame.h"

#include "nuis/log.h"

#include <functional>
#include <numeric>

namespace nuis {

class FrameGen : public nuis_named_log("Frame") {

public:
  using FilterFunc = std::function<int(HepMC3::GenEvent const &)>;
  using ProjectionFunc = std::function<double(HepMC3::GenEvent const &)>;
  using ProjectionsFunc =
      std::function<std::vector<double>(HepMC3::GenEvent const &)>;

  // PS An option to input a vector of functions is also needed (instead of
  // requiring a lambda to build it) LP Is add_column("name",
  // func).add_column("name", func).add_column("name", func) not okay?

  FrameGen(INormalizedEventSourcePtr evs, size_t block_size = 500000);

  FrameGen filter(FilterFunc filt);
  FrameGen add_columns(std::vector<std::string> col_names,
                       ProjectionsFunc proj);
  FrameGen add_column(std::string col_name, ProjectionFunc proj);
  FrameGen limit(size_t nmax);
  FrameGen progress(size_t every = 100000);

  Frame first();
  Frame next();
  Frame all();

private:
  INormalizedEventSourcePtr source;

  std::vector<FilterFunc> filters;

  struct HeadedColumnProjectors {
    std::vector<std::string> Head;
    ProjectionsFunc Proj;
  };

  std::vector<HeadedColumnProjectors> projections;

  size_t chunk_size;

  size_t max_events_to_loop;
  size_t progress_report_every;
  size_t nevents;

  size_t GetNCols();
  
  // first/next state
  std::vector<std::string> column_names;
  size_t n_total_rows;
  size_t neventsprocessed;
  INormalizedEventSource_looper ev_it;
  NormInfo norm_info;
};

} // namespace nuis