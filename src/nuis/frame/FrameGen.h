#pragma once

#include "nuis/frame/Frame.h"

#include <functional>
#include <numeric>

namespace nuis {

class FrameGen {

public:
  using FilterFunc = std::function<bool(HepMC3::GenEvent const &)>;
  using ProjectionFunc = std::function<double(HepMC3::GenEvent const &)>;
  using ProjectionsFunc =
      std::function<std::vector<double>(HepMC3::GenEvent const &)>;

  FrameGen(INormalizedEventSourcePtr evs, size_t block_size = 50000);

  FrameGen Filter(FilterFunc filt);
  FrameGen AddColumns(std::vector<std::string> col_names, ProjectionsFunc proj);
  FrameGen AddColumn(std::string col_name, ProjectionFunc proj);
  FrameGen Limit(size_t nmax);

  Frame Evaluate();

private:
  INormalizedEventSourcePtr source;

  std::vector<FilterFunc> filters;

  struct HeadedColumnProjectors {
    std::vector<std::string> Head;
    ProjectionsFunc Proj;
  };

  std::vector<HeadedColumnProjectors> projections;

  size_t chunk_size;
  std::vector<Eigen::MatrixXd> chunks;

  size_t max_events_to_loop;

  size_t GetNCols();
};

} // namespace nuis