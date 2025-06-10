#include "nuis/python/pyEventFrame.h"

#include "nuis/log.txx"

#ifdef NUIS_ARROW_ENABLED
// header that
#include "arrow/python/pyarrow.h"
#endif

#include "nuis/eventframe/utility.h"

#include "ProSelecta/env.h"

#include "HepMC3/Print.h"

namespace py = pybind11;

using namespace nuis;

pyEventFrameGen::pyEventFrameGen(pyNormalizedEventSource ev, size_t bsize) {
  gen = std::make_shared<EventFrameGen>(ev.evs, bsize);
}

pyEventFrameGen pyEventFrameGen::filter(EventFrameGen::FilterFunc filt) {
  *gen = gen->filter(filt);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_bool_columns(std::vector<std::string> const &col_names,
                                  EventFrameGen::ProjectionsFunc<bool> proj) {
  *gen = gen->add_typed_columns<bool>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_bool_column(std::string const &col_name,
                                 EventFrameGen::ProjectionFunc<bool> proj) {
  *gen = gen->add_typed_column<bool>(col_name, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_int_columns(std::vector<std::string> const &col_names,
                                 EventFrameGen::ProjectionsFunc<int> proj) {
  *gen = gen->add_typed_columns<int>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_int_column(std::string const &col_name,
                                EventFrameGen::ProjectionFunc<int> proj) {
  *gen = gen->add_typed_column<int>(col_name, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_uint_columns(std::vector<std::string> const &col_names,
                                  EventFrameGen::ProjectionsFunc<uint> proj) {
  *gen = gen->add_typed_columns<uint>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_uint_column(std::string const &col_name,
                                 EventFrameGen::ProjectionFunc<uint> proj) {
  *gen = gen->add_typed_column<uint>(col_name, proj);
  return *this;
}

pyEventFrameGen pyEventFrameGen::add_int16_columns(
    std::vector<std::string> const &col_names,
    EventFrameGen::ProjectionsFunc<int16_t> proj) {
  *gen = gen->add_typed_columns<int16_t>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_int16_column(std::string const &col_name,
                                  EventFrameGen::ProjectionFunc<int16_t> proj) {
  *gen = gen->add_typed_column<int16_t>(col_name, proj);
  return *this;
}

pyEventFrameGen pyEventFrameGen::add_uint16_columns(
    std::vector<std::string> const &col_names,
    EventFrameGen::ProjectionsFunc<uint16_t> proj) {
  *gen = gen->add_typed_columns<uint16_t>(col_names, proj);
  return *this;
}

pyEventFrameGen pyEventFrameGen::add_uint16_column(
    std::string const &col_name, EventFrameGen::ProjectionFunc<uint16_t> proj) {
  *gen = gen->add_typed_column<uint16_t>(col_name, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_float_columns(std::vector<std::string> const &col_names,
                                   EventFrameGen::ProjectionsFunc<float> proj) {
  *gen = gen->add_typed_columns<float>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_float_column(std::string const &col_name,
                                  EventFrameGen::ProjectionFunc<float> proj) {
  *gen = gen->add_typed_column<float>(col_name, proj);
  return *this;
}

pyEventFrameGen pyEventFrameGen::add_double_columns(
    std::vector<std::string> const &col_names,
    EventFrameGen::ProjectionsFunc<double> proj) {
  *gen = gen->add_typed_columns<double>(col_names, proj);
  return *this;
}

pyEventFrameGen
pyEventFrameGen::add_double_column(std::string const &col_name,
                                   EventFrameGen::ProjectionFunc<double> proj) {
  *gen = gen->add_typed_column<double>(col_name, proj);
  return *this;
}

pyEventFrameGen pyEventFrameGen::limit(size_t nmax) {
  *gen = gen->limit(nmax);
  return *this;
}

pyEventFrameGen pyEventFrameGen::progress(size_t nmax) {
  *gen = gen->progress(nmax);
  return *this;
}

nuis::EventFrame pyEventFrameGen::first(size_t nchunk) {
  return gen->first(nchunk);
}
nuis::EventFrame pyEventFrameGen::next(size_t nchunk) {
  return gen->next(nchunk);
}
nuis::EventFrame pyEventFrameGen::all() { return gen->all(); }

#ifdef NUIS_ARROW_ENABLED
pybind11::object pyEventFrameGen::firstArrow(size_t nchunk) {

  decltype(gen->firstArrow(nchunk)) rb;
  try {
    rb = gen->firstArrow(nchunk);
  } catch (FrameGenerationException const &ex) {
    log_error(
        "pyEventFrameGen::firstArrow failed processing on event, mode: {}",
        ps::event::signal_process_id(gen->get_error_event()));
    HepMC3::Print::content(gen->get_error_event());
    return py::none();
  }
  if (rb) {
    return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
  }
  return py::none();
}
pybind11::object pyEventFrameGen::nextArrow(size_t nchunk) {
  decltype(gen->nextArrow(nchunk)) rb;

  try {
    rb = gen->nextArrow(nchunk);
  } catch (FrameGenerationException const &ex) {
    log_error("pyEventFrameGen::nextArrow failed processing on event, mode: {}",
              ps::event::signal_process_id(gen->get_error_event()));
    HepMC3::Print::content(gen->get_error_event());
    return py::none();
  }
  if (rb) {
    return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
  }
  return py::none();
}
#endif

Eigen::ArrayXdRef frame_gettattr(EventFrame &s, std::string const &column) {
  return s.col(column);
}

void frame_settattr(EventFrame &s, std::string const &column,
                    Eigen::ArrayXdRef const data) {
  auto cid = s.find_column_index(column);
  if (cid != EventFrame::npos) {
    s.table.col(cid) = data;
  }
}

void pyEventFrameInit(py::module &m) {

#ifdef NUIS_ARROW_ENABLED
  arrow::py::import_pyarrow();
  m.add_object("pa", py::module::import("pyarrow"));
#endif

  py::class_<EventFrame>(m, "EventFrame")
      .def(py::init<>())
      .def_readwrite("table", &EventFrame::table,
                     py::return_value_policy::reference_internal)
      .def_readwrite("column_names", &EventFrame::column_names,
                     py::return_value_policy::reference_internal)
      .def_readonly("num_rows", &EventFrame::num_rows)
      .def("__bool__", [](EventFrame const &s) { return bool(s.table.rows()); })
      .def("find_column_index", &EventFrame::find_column_index)
      .def("topRows", &EventFrame::topRows)
      .def("bottomRows", &EventFrame::bottomRows)
      .def_readonly_static("npos", &EventFrame::npos)
      .def_readonly_static("missing_datum", &kMissingDatum<double>)
      .def("__getattr__", &frame_gettattr)
      .def("__setattr__", &frame_settattr)
      .def("__getitem__", &frame_gettattr)
      .def("__setitem__", &frame_settattr)
      .def("__str__", &str_via_ss<EventFrame>)
      .def_static(
          "get_best_fatx_per_sumw_estimate",
          [](EventFrame const &ef,
             NuHepMC::CrossSection::Units::Unit const &units) {
            return get_best_fatx_per_sumw_estimate(ef, units);
          },
          py::arg("evf"),
          py::arg("units") = NuHepMC::CrossSection::Units::cm2ten38_PerNucleon)
#ifdef NUIS_ARROW_ENABLED
      .def_static(
          "get_best_fatx_per_sumw_estimate",
          [](py::handle pyarrobj,
             NuHepMC::CrossSection::Units::Unit const &units) {
            if (arrow::py::is_table(pyarrobj.ptr())) {
              return get_best_fatx_per_sumw_estimate(
                  arrow::py::unwrap_table(pyarrobj.ptr()).ValueOrDie(), units);
            } else if (arrow::py::is_batch(pyarrobj.ptr())) {
              return get_best_fatx_per_sumw_estimate(
                  arrow::py::unwrap_batch(pyarrobj.ptr()).ValueOrDie(), units);
            } else {
              throw std::runtime_error(
                  "Invalid python type passed to "
                  "get_best_fatx_per_sumw_estimate. Expected pyarrow "
                  "RecordBatch or Table.");
            }
          },
          py::arg("evf"),
          py::arg("units") = NuHepMC::CrossSection::Units::cm2ten38_PerNucleon)
#endif
      ;

  py::class_<pyEventFrameGen>(m, "EventFrameGen")
      .def(py::init<pyNormalizedEventSource, size_t>(), py::arg("event_source"),
           py::arg("block_size") = 250000)
      .def("filter", &pyEventFrameGen::filter)
#ifdef NUIS_ARROW_ENABLED
      .def_static("has_arrow_support", []() { return true; })
#else
      .def_static("has_arrow_support", []() { return false; })
#endif
      .def("add_column", &pyEventFrameGen::add_double_column)
      .def("add_columns", &pyEventFrameGen::add_double_columns)
      .def("add_bool_column", &pyEventFrameGen::add_bool_column)
      .def("add_bool_columns", &pyEventFrameGen::add_bool_columns)
      .def("add_int_column", &pyEventFrameGen::add_int_column)
      .def("add_int_columns", &pyEventFrameGen::add_int_columns)
      .def("add_uint_column", &pyEventFrameGen::add_uint_column)
      .def("add_uint_columns", &pyEventFrameGen::add_uint_columns)
      .def("add_int16_column", &pyEventFrameGen::add_int16_column)
      .def("add_int16_columns", &pyEventFrameGen::add_int16_columns)
      .def("add_uint16_column", &pyEventFrameGen::add_uint16_column)
      .def("add_uint16_columns", &pyEventFrameGen::add_uint16_columns)
      .def("add_double_column", &pyEventFrameGen::add_double_column)
      .def("add_double_columns", &pyEventFrameGen::add_double_columns)
      .def("add_float_column", &pyEventFrameGen::add_float_column)
      .def("add_float_columns", &pyEventFrameGen::add_float_columns)
      .def("limit", &pyEventFrameGen::limit)
      .def("limit", [](pyEventFrameGen &s, double i) { return s.limit(i); })
      .def("progress", &pyEventFrameGen::progress, py::arg("every") = 100000)
      .def("first", &pyEventFrameGen::first,
           py::arg("nchunk") = std::numeric_limits<size_t>::max())
      .def("next", &pyEventFrameGen::next,
           py::arg("nchunk") = std::numeric_limits<size_t>::max())
#ifdef NUIS_ARROW_ENABLED
      .def("firstArrow", &pyEventFrameGen::firstArrow,
           py::arg("nchunk") = std::numeric_limits<size_t>::max())
      .def("nextArrow", &pyEventFrameGen::nextArrow,
           py::arg("nchunk") = std::numeric_limits<size_t>::max())
#endif
      .def("all", &pyEventFrameGen::all);
}
