#include "nuis/frame/FrameGen.h"

namespace nuis {

FrameGen::FrameGen(INormalizedEventSourcePtr evs, size_t block_size)
    : source(evs), chunk_size{block_size} {}

FrameGen FrameGen::Filter(FilterFunc filt) {
  filters.push_back(filt);
  return *this;
}
FrameGen FrameGen::AddColumns(std::vector<std::string> col_names,
                              ProjectionsFunc proj) {
  projections.push_back(HeadedColumnProjectors{col_names, proj});
  return *this;
}
FrameGen FrameGen::AddColumn(std::string col_name, ProjectionFunc proj) {
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
FrameGen FrameGen::Limit(size_t nmax) {
  max_events_to_loop = nmax;
  return *this;
}

Frame FrameGen::Evaluate() {
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
    bool cut = false;
    for (auto &filt : filters) {
      if (!filt(ev)) {
        cut = true;
        break;
      }
    }
    if (cut) {
      // have to do this before the next loop otherwise we read one too many
      // events
      if (++neventsprocessed >= max_events_to_loop) {
        break;
      }
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

    // have to do this before the next loop otherwise we read one too many
    // events
    if (++neventsprocessed >= max_events_to_loop) {
      break;
    }
  }

  // big allocation
  Frame out{column_names, Eigen::MatrixXd(row, column_names.size()),
            source->norm_info()};

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

size_t FrameGen::GetNCols() {
  return 2 + std::accumulate(projections.begin(), projections.end(), 0,
                             [](int acc, auto const &hcp) {
                               return acc + hcp.Head.size();
                             });
}

} // namespace nuis