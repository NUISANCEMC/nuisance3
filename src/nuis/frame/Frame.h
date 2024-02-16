#pragma once

#include "Eigen/Dense"

#include <functional>
#include <numeric>
#include <string>
#include <vector>

namespace nuis {

struct Frame {
  std::vector<std::string> ColumnNames;
  Eigen::MatrixXd Table;
};

class FrameGen {

public:
  using FilterFunc = std::function<bool(HepMC3::GenEvent const &)>;
  using ProjectionFunc = std::function<double(HepMC3::GenEvent const &)>;
  using ProjectionsFunc =
      std::function<std::vector<double>(HepMC3::GenEvent const &)>;

  FrameGen(INormalizedEventSourcePtr evs, size_t block_size = 1000)
      : source(evs), chunk_size{block_size} {}

  FrameGen Filter(FilterFunc filt) {
    filters.push_back(filt);
    return *this;
  }
  FrameGen AddColumns(std::vector<std::string> col_names,
                      ProjectionsFunc proj) {
    projections.push_back(HeadedColumnProjectors{col_names, proj});
    return *this;
  }
  FrameGen AddColumn(std::string col_name, ProjectionFunc proj) {
    AddColumns(
        {
            col_name,
        },
        [=](auto const &ev) -> std::vector<double> {
          return {
              proj(ev),
          };
        });
    return *this;
  }
  FrameGen Limit(size_t nmax) {
    max_events_to_loop = nmax;
    return *this;
  }

  Frame Evaluate() {
    auto column_names =
        std::accumulate(projections.begin(), projections.end(),
                        std::vector<std::string>{"evt_number", "cvw"},
                        [](auto cols, auto const &hcp) {
                          for (auto h : hcp.Head) {
                            cols.push_back(h);
                          }
                          return cols;
                        });

    chunks.emplace_back(chunk_size, column_names.size());

    size_t row = 0;
    size_t neventsprocessed = 0;
    for (auto const &[ev, cvw] : source) {
      if (neventsprocessed++ >= max_events_to_loop) {
        break;
      }

      bool cut = false;
      for (auto &filt : filters) {
        if (!filt(ev)) {
          cut = true;
          break;
        }
      }
      if (cut) {
        continue;
      }

      size_t chunk_id = row / chunk_size;
      size_t chunk_row = row % chunk_size;

      if (chunk_id >= chunks.size()) {
        chunks.emplace_back(chunk_size, column_names.size());
      }

      chunks[chunk_id](chunk_row, 0) = ev.event_number();
      chunks[chunk_id](chunk_row, 1) = cvw;

      size_t col_id = 2;
      for (auto &[head, proj] : projections) {
        auto projs = proj(ev);
        for (size_t i = 0; i < head.size(); ++i) {

          if (i < projs.size()) {
            chunks[chunk_id](chunk_row, col_id) = projs[i];
          } else {
            chunks[chunk_id](chunk_row, col_id) = 0xdeadbeef;
          }

          col_id++;
        }
      }

      row++;
    }

    // big allocation
    Frame out{column_names, Eigen::MatrixXd(row, column_names.size())};

    size_t rows_copied = 0;
    for (auto const &chunk : chunks) {

      size_t start_row = rows_copied;

      size_t nrows_in_chunk =
          (rows_copied + chunk_size) > row ? (row % chunk_size) : chunk_size;

      size_t end_row = rows_copied + nrows_in_chunk;

      out.Table.middleRows(start_row, end_row) =
          chunk.middleRows(0, nrows_in_chunk);

      rows_copied += nrows_in_chunk;
    }

    chunks.clear();

    return out;
  }

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

  size_t GetNCols() {
    return 2 + std::accumulate(projections.begin(), projections.end(), 0,
                               [](int acc, auto const &hcp) {
                                 return acc + hcp.Head.size();
                               });
  }
};

} // namespace nuis