#include "nuis/frame/FrameGen.h"

#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/chrono.h"

#include "nuis/log.txx"

namespace nuis {

FrameGen::FrameGen(INormalizedEventSourcePtr evs, size_t block_size)
    : source(evs), chunk_size{block_size},
      max_events_to_loop{std::numeric_limits<size_t>::max()},
      progress_report_every{std::numeric_limits<size_t>::max()},
      nevents{std::numeric_limits<size_t>::max()}, ev_it(nullptr) {
  auto run_info = evs->first().value().evt->run_info();
  if (run_info && NuHepMC::GC1::SignalsConvention(run_info, "G.C.2")) {
    nevents = NuHepMC::GC2::ReadExposureNEvents(run_info);
  }
}

FrameGen FrameGen::filter(FilterFunc filt) {
  filters.push_back(filt);
  return *this;
}
FrameGen FrameGen::limit(size_t nmax) {
  max_events_to_loop = nmax;
  return *this;
}
FrameGen FrameGen::progress(size_t every) {
  progress_report_every = every;
  set_log_level(log_level::info);
  return *this;
}

Frame FrameGen::first() {
  all_column_names = std::accumulate(columns.begin(), columns.end(),
                                     std::vector<std::string>{"evt#", "cvw"},
                                     [](auto cols, auto const &hcp) {
                                       for (auto h : hcp.column_names) {
                                         cols.push_back(h);
                                       }
                                       return cols;
                                     });

  n_total_rows = 0;
  neventsprocessed = 0;
  ev_it = begin(source);

  return next();
}

template <typename T>
size_t FrameGen::fill_row_columns(Eigen::ArrayXdRef row,
                                  HepMC3::GenEvent const &ev, size_t proj_index,
                                  size_t first_col) {
  auto const &projs = get_proj_functions<T>()[proj_index](ev);

  for (size_t i = 0; i < projs.size(); ++i) {
    row[first_col++] = projs[i];
  }

  return first_col;
}

Frame FrameGen::next() {
  log_trace("FrameGen::next() neventsprocessed: {}, max_events_to_loop: {}",
            neventsprocessed, max_events_to_loop);

  if (neventsprocessed >= max_events_to_loop) {
    return {all_column_names, Eigen::ArrayXXd(0, all_column_names.size()),
            norm_info};
  }

  Eigen::ArrayXXd chunk(chunk_size, all_column_names.size());

  size_t chunk_row = 0;

  auto end_it = end(source);

  auto nmaxloop = std::min(max_events_to_loop, nevents);

  while (ev_it != end_it) {
    auto const &[evp, cvw] = *ev_it;
    auto const &ev = *evp;

    NUIS_LOG_TRACE("FrameGen::next() chunk_row: {} ", chunk_row);

    if (neventsprocessed && progress_report_every &&
        !(neventsprocessed % progress_report_every)) {
      log_info("FrameGen has selected {}{} from {} processed events.",
               n_total_rows,
               ((nmaxloop != std::numeric_limits<size_t>::max())
                    ? fmt::format("/{}", nmaxloop)
                    : ""),
               neventsprocessed);
    }

    bool cut = false;
    for (auto &filt : filters) {
      if (!filt(ev)) {
        NUIS_LOG_TRACE("FrameGen::next() chunk_row: {} was cut ", chunk_row);
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

    NUIS_LOG_TRACE("FrameGen::next() chunk_row: {} was kept, event_number: {} ",
                   ev.event_number());

    size_t col_id = 2;
    for (auto &[column_names, typenum, proj_index] : columns) {
      size_t next_col_id = col_id;

      switch (typenum) {
      case column_type<int>::id:
        next_col_id =
            fill_row_columns<int>(chunk.row(chunk_row), ev, proj_index, col_id);
        break;
      case column_type<double>::id:
        next_col_id = fill_row_columns<double>(chunk.row(chunk_row), ev,
                                               proj_index, col_id);
        break;
      }

      for (size_t i = next_col_id; i < (col_id + column_names.size()); ++i) {
        chunk(chunk_row, i) = kMissingDatum<double>;
      }
      col_id += column_names.size();
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

  log_trace("FrameGen::next() done looping  n_total_rows: {} neventsprocessed: "
            "{} chunk_row: {}",
            n_total_rows, neventsprocessed, chunk_row);

  // grab the norm_info before reading the next event for the start of the next
  // loop
  norm_info = source->norm_info();
  ++ev_it;
  return {all_column_names, chunk.topRows(chunk_row), norm_info};
}

Frame FrameGen::all() {

  log_info("FrameGen::all Chunk shape: {} rows {} cols, {} KB.", chunk_size,
           GetNCols(), ((chunk_size * GetNCols()) * sizeof(double)) / 1024);

  Eigen::ArrayXXd builder;
  Eigen::ArrayXXd next_chunk = first().table;
  log_trace("FrameGen::all() first with nrows {}", builder.rows());

  size_t last_report_size = 0;
  if ((neventsprocessed - last_report_size) > progress_report_every) {
    log_info("FrameGen::all() is using ~{} MB of memory. Output frame will "
             "be {} MB.",
             ((builder.size() * 2 + next_chunk.size()) * sizeof(double)) /
                 (1024 * 1024),
             (builder.size() * sizeof(double)) / (1024 * 1024));
    last_report_size = builder.rows() + next_chunk.rows();
  }

  while (next_chunk.rows()) {
    log_trace("FrameGen::all() got chunk with nrows {}", next_chunk.rows());

    Eigen::ArrayXXd new_builder(builder.rows() + next_chunk.rows(),
                                next_chunk.cols());
    new_builder.topRows(builder.rows()) = builder;
    new_builder.bottomRows(next_chunk.rows()) = next_chunk;

    builder = new_builder;

    next_chunk = next().table;

    if ((neventsprocessed - last_report_size) > progress_report_every) {
      log_info("FrameGen::all() is using ~{} MB of memory. Output frame will "
               "be {} MB.",
               ((builder.size() * 2 + next_chunk.size()) * sizeof(double)) /
                   (1024 * 1024),
               (builder.size() * sizeof(double)) / (1024 * 1024));
      last_report_size = builder.rows() + next_chunk.rows();
    }
  }

  log_trace("FrameGen::all() done: nrows {}", builder.rows());

  return {all_column_names, builder, source->norm_info()};
}

#ifdef NUIS_ARROW_ENABLED

template <typename T>
std::pair<std::shared_ptr<arrow::Field>, std::unique_ptr<arrow::ArrayBuilder>>
GetColumnBuilder(std::string const &name) {
  return {arrow::field(name, nuis::column_type<T>::mkt()),
          nuis::column_type<T>::mkb()};
}

template <typename T>
typename nuis::column_type<T>::ATT::BuilderType &
BuilderAs(std::unique_ptr<arrow::ArrayBuilder> &ab) {
  return *dynamic_cast<typename nuis::column_type<T>::ATT::BuilderType *>(
      ab.get());
}

template <typename T>
void FrameGen::fill_array_builder(
    std::vector<std::unique_ptr<arrow::ArrayBuilder>> &array_builders,
    HepMC3::GenEvent const &ev, size_t proj_index, size_t first_col,
    size_t ncols_to_fill) {

  size_t next_col_id = first_col;

  auto const &projs = get_proj_functions<T>()[proj_index](ev);
  for (size_t i = 0; i < std::min(projs.size(), ncols_to_fill); ++i) {
    BuilderAs<T>(array_builders[next_col_id++]).Append(projs[i]);
  }

  for (size_t i = next_col_id; i < (first_col + ncols_to_fill); ++i) {
    BuilderAs<T>(array_builders[i]).Append(kMissingDatum<T>);
  }
}

std::shared_ptr<arrow::RecordBatch> FrameGen::firstArrow() {
  all_column_names = std::accumulate(
      columns.begin(), columns.end(),
      std::vector<std::string>{"event number", "cv weight", "fatx estimate"},
      [](auto cols, auto const &hcp) {
        for (auto h : hcp.column_names) {
          cols.push_back(h);
        }
        return cols;
      });

  n_total_rows = 0;
  neventsprocessed = 0;
  ev_it = begin(source);

  return _nextArrow().ValueOrDie();
}

arrow::Result<std::shared_ptr<arrow::RecordBatch>> FrameGen::_nextArrow() {
  log_trace(
      "FrameGen::nextArrow() neventsprocessed: {}, max_events_to_loop: {}",
      neventsprocessed, max_events_to_loop);

  if (neventsprocessed >= max_events_to_loop) {
    return arrow::Result(nullptr);
  }

  std::vector<std::shared_ptr<arrow::Field>> schema_list;
  std::vector<std::unique_ptr<arrow::ArrayBuilder>> array_builders;

  log_debug("FrameGen::nextArrow() Building schema: ");

  for (auto const &[name, typenum] : std::vector<std::pair<std::string, int>>{
           {"event number", nuis::column_type<int>::id},
           {"cv weight", nuis::column_type<double>::id},
           {"fatx estimate", nuis::column_type<double>::id}}) {

    std::shared_ptr<arrow::Field> col = nullptr;
    std::unique_ptr<arrow::ArrayBuilder> builder = nullptr;

    log_debug("\t\t col: {}, name: {}, type: {}", schema_list.size(), name,
              column_typenum_as_string(typenum));

    if (typenum == nuis::column_type<int>::id) {
      std::tie(col, builder) = GetColumnBuilder<int>(name);
    } else if (typenum == nuis::column_type<double>::id) {
      std::tie(col, builder) = GetColumnBuilder<double>(name);
    }

    schema_list.push_back(std::move(col));
    array_builders.push_back(std::move(builder));
  }

  for (auto &[column_names, typenum, proj_index] : columns) {
    for (auto const &name : column_names) {
      std::shared_ptr<arrow::Field> col = nullptr;
      std::unique_ptr<arrow::ArrayBuilder> builder = nullptr;

      if (typenum == nuis::column_type<int>::id) {
        std::tie(col, builder) = GetColumnBuilder<int>(name);
      } else if (typenum == nuis::column_type<double>::id) {
        std::tie(col, builder) = GetColumnBuilder<double>(name);
      }

      log_debug(
          "\t\t col: {}, name: {}, type: {}, field: {}, array_builder: {}",
          schema_list.size(), name, column_typenum_as_string(typenum),
          static_cast<void *>(col.get()), static_cast<void *>(builder.get()));

      schema_list.push_back(std::move(col));
      array_builders.push_back(std::move(builder));
    }
  }

  auto schema = std::make_shared<arrow::Schema>(schema_list);

  size_t rbatch_row = 0;
  auto end_it = end(source);
  auto nmaxloop = std::min(max_events_to_loop, nevents);

  while (ev_it != end_it) {
    auto const &[evp, cvw] = *ev_it;
    auto const &ev = *evp;

    NUIS_LOG_TRACE("FrameGen::nextArrow() rbatch_row: {} ", rbatch_row);

    if (neventsprocessed && progress_report_every &&
        !(neventsprocessed % progress_report_every)) {
      log_info("FrameGen has selected {}{} from {} processed events.",
               n_total_rows,
               ((nmaxloop != std::numeric_limits<size_t>::max())
                    ? fmt::format("/{}", nmaxloop)
                    : ""),
               neventsprocessed);
    }

    bool cut = false;
    for (auto &filt : filters) {
      if (!filt(ev)) {
        NUIS_LOG_TRACE("FrameGen::nextArrow() event: {} was cut ",
                       neventsprocessed);
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

    NUIS_LOG_TRACE(
        "FrameGen::nextArrow() rbatch_row: {} was kept, event_number: {} ",
        ev.event_number());

    BuilderAs<int>(array_builders[0]).Append(ev.event_number());
    BuilderAs<double>(array_builders[1]).Append(cvw);

    auto [fatx, sumweights, nevents] = source->norm_info();
    BuilderAs<double>(array_builders[2]).Append(fatx);

    size_t col_id = 3;
    for (auto &[column_names, typenum, proj_index] : columns) {

      switch (typenum) {
      case column_type<int>::id:
        fill_array_builder<int>(array_builders, ev, proj_index, col_id,
                                column_names.size());
        break;
      case column_type<double>::id:
        fill_array_builder<double>(array_builders, ev, proj_index, col_id,
                                   column_names.size());
        break;
      }

      col_id += column_names.size();
    }

    n_total_rows++;
    neventsprocessed++;
    rbatch_row++;
    // have to do this before the next loop otherwise we read one too many
    // events
    if (neventsprocessed >= max_events_to_loop) {
      break;
    }
    if (rbatch_row >= chunk_size) {
      break;
    }
    ++ev_it;
  }

  log_trace(
      "FrameGen::nextArrow() done looping  n_total_rows: {} neventsprocessed: "
      "{} rbatch_row: {}",
      n_total_rows, neventsprocessed, rbatch_row);

  // grab the norm_info before reading the next event for the start of the next
  // loop
  norm_info = source->norm_info();
  ++ev_it;

  std::vector<std::shared_ptr<arrow::Array>> arrays(all_column_names.size(),
                                                    nullptr);
  ARROW_ASSIGN_OR_RAISE(arrays[0], BuilderAs<int>(array_builders[0]).Finish());
  ARROW_ASSIGN_OR_RAISE(arrays[1],
                        BuilderAs<double>(array_builders[1]).Finish());
  ARROW_ASSIGN_OR_RAISE(arrays[2],
                        BuilderAs<double>(array_builders[2]).Finish());
  log_debug("FrameGen::nextArrow() building arrays: ");
  log_debug("\t\t col: {}, name: {}, type: {}, num: {}", 0, all_column_names[0],
            column_typenum_as_string(column_type<int>::id),
            arrays[0]->length());
  log_debug("\t\t col: {}, name: {}, type: {}, num: {}", 1, all_column_names[1],
            column_typenum_as_string(column_type<double>::id),
            arrays[1]->length());
  log_debug("\t\t col: {}, name: {}, type: {}, num: {}", 2, all_column_names[2],
            column_typenum_as_string(column_type<double>::id),
            arrays[2]->length());

  size_t col_id = 3;
  for (auto &[column_names, typenum, proj_index] : columns) {

    switch (typenum) {
    case column_type<int>::id:
      for (size_t i = 0; i < column_names.size(); ++i) {
        ARROW_ASSIGN_OR_RAISE(
            arrays[col_id + i],
            BuilderAs<int>(array_builders[col_id + i]).Finish());
      }
      break;
    case column_type<double>::id:
      for (size_t i = 0; i < column_names.size(); ++i) {
        ARROW_ASSIGN_OR_RAISE(
            arrays[col_id + i],
            BuilderAs<double>(array_builders[col_id + i]).Finish());
      }
      break;
    }

    for (size_t i = 0; i < column_names.size(); ++i) {
      log_debug("\t\t col: {}, name: {}, type: {}, num: {}", col_id + i,
                all_column_names[col_id + i], column_typenum_as_string(typenum),
                arrays[col_id + i]->length());
    }

    col_id += column_names.size();
  }

  return arrow::Result(
      arrow::RecordBatch::Make(schema, arrays.front()->length(), arrays));
}

std::shared_ptr<arrow::RecordBatch> FrameGen::nextArrow() {
  return _nextArrow().ValueOrDie();
}

#endif

size_t FrameGen::GetNCols() {
  return 2 + std::accumulate(columns.begin(), columns.end(), 0,
                             [](int acc, auto const &hcp) {
                               return acc + hcp.column_names.size();
                             });
}

} // namespace nuis