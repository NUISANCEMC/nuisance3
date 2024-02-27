#include "nuis/frame/FrameGen.h"

#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/chrono.h"

#include "spdlog/spdlog.h"

namespace nuis {

FrameGen::FrameGen(INormalizedEventSourcePtr evs, size_t block_size)
    : source(evs), chunk_size{block_size},
      progress_report_every{std::numeric_limits<size_t>::max()},
      nevents{std::numeric_limits<size_t>::max()}, ev_it(nullptr) {
  auto run_info = evs->first().value().evt.run_info();
  if (run_info && NuHepMC::GC1::SignalsConvention(run_info, "G.C.2")) {
    nevents = NuHepMC::GC2::ReadExposureNEvents(run_info);
  }
}

FrameGen FrameGen::filter(FilterFunc filt) {
  filters.push_back(filt);
  return *this;
}
FrameGen FrameGen::add_columns(std::vector<std::string> col_names,
                               ProjectionsFunc proj) {
  projections.push_back(HeadedColumnProjectors{col_names, proj});
  return *this;
}
FrameGen FrameGen::add_column(std::string col_name, ProjectionFunc proj) {
  add_columns(
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
FrameGen FrameGen::limit(size_t nmax) {
  max_events_to_loop = nmax;
  return *this;
}
FrameGen FrameGen::progress(size_t every) {
  progress_report_every = every;
  return *this;
}

Frame FrameGen::first() {
  column_names = std::accumulate(projections.begin(), projections.end(),
                                 std::vector<std::string>{"evt#", "cvw"},
                                 [](auto cols, auto const &hcp) {
                                   for (auto h : hcp.Head) {
                                     cols.push_back(h);
                                   }
                                   return cols;
                                 });

  n_total_rows = 0;
  neventsprocessed = 0;
  ev_it = begin(source);

  return next();
}

Frame FrameGen::next() {

  if (neventsprocessed >= max_events_to_loop) {
    return {column_names, Eigen::ArrayXXd(0, column_names.size()), norm_info};
  }

  Eigen::ArrayXXd chunk(chunk_size, column_names.size());

  size_t chunk_row = 0;

  auto end_it = end(source);

  auto nmaxloop = std::min(max_events_to_loop, nevents);

  while (ev_it != end_it) {
    auto const &[ev, cvw] = *ev_it;

    if (neventsprocessed && progress_report_every &&
        !(neventsprocessed % progress_report_every)) {
      spdlog::info("GenFrame has selected {}{} from {} processed events.",
                   n_total_rows,
                   ((nmaxloop != std::numeric_limits<size_t>::max())
                        ? fmt::format("/{}", nmaxloop)
                        : ""),
                   neventsprocessed);
    }

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
      ++ev_it;
      continue;
    }

    chunk(chunk_row, 0) = ev.event_number();
    chunk(chunk_row, 1) = cvw;

    size_t col_id = 2;
    for (auto &[head, proj] : projections) {
      auto projs = proj(ev);
      for (size_t i = 0; i < head.size(); ++i) {

        if (i < projs.size()) {
          chunk(chunk_row, col_id) = projs[i];
        } else {
          chunk(chunk_row, col_id) = Frame::missing_datum;
        }

        col_id++;
      }
    }

    n_total_rows++;
    neventsprocessed++;
    chunk_row++;
    // have to do this before the next loop otherwise we read one too many
    // events
    if (neventsprocessed >= max_events_to_loop) {
      break;
    }
    if (chunk_row >= chunk_size) {
      break;
    }
    ++ev_it;
  }
  // grab the norm_info before reading the next event for the start of the next
  // loop
  norm_info = source->norm_info();
  ++ev_it;
  return {column_names, chunk.topRows(chunk_row), norm_info};
}

Frame FrameGen::all() {

  Eigen::ArrayXXd builder = first().table;
  Eigen::ArrayXXd next_chunk = next().table;

  while (next_chunk.rows()) {
    Eigen::ArrayXXd new_builder(builder.rows() + next_chunk.rows(),
                                builder.cols());
    new_builder.topRows(builder.rows()) = builder;
    new_builder.bottomRows(next_chunk.rows()) = next_chunk;

    next_chunk = next().table;
  }

  return {column_names, builder, source->norm_info()};
}

size_t FrameGen::GetNCols() {
  return 2 + std::accumulate(projections.begin(), projections.end(), 0,
                             [](int acc, auto const &hcp) {
                               return acc + hcp.Head.size();
                             });
}

} // namespace nuis