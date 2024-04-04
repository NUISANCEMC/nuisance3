#include "nuis/python/pyEventFrame.h"

#ifdef NUIS_ARROW_ENABLED
// header that
#include "arrow/python/pyarrow.h"
#endif

namespace py = pybind11;

using namespace nuis;

pyEventFrameGen::pyEventFrameGen(pyNormalizedEventSource ev, size_t bsize) {
  gen = std::make_shared<EventFrameGen>(ev.evs, bsize);
}

pyEventFrameGen pyEventFrameGen::filter(EventFrameGen::FilterFunc filt) {
  *gen = gen->filter(filt);
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
  auto rb = gen->firstArrow(nchunk);
  if (rb) {
    return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
  }
  return py::none();
}
pybind11::object pyEventFrameGen::nextArrow(size_t nchunk) {
  auto rb = gen->nextArrow(nchunk);
  if (rb) {
    return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
  }
  return py::none();
}
#endif

NormInfo pyEventFrameGen::norm_info() const { return gen->norm_info(); }

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
      .def("norm_info", [](EventFrame const &s) { return s.norm_info; })
      .def_readonly("num_rows", &EventFrame::num_rows)
      .def("__bool__", [](EventFrame const &s) { return bool(s.table.rows()); })
      .def("find_column_index", &EventFrame::find_column_index)
      .def_readonly_static("npos", &EventFrame::npos)
      .def_readonly_static("missing_datum", &kMissingDatum<double>)
      .def("__getattr__", &frame_gettattr)
      .def("__setattr__", &frame_settattr)
      .def("__getitem__", &frame_gettattr)
      .def("__setitem__", &frame_settattr)
      .def("__str__", &str_via_ss<EventFrame>);

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
      .def("add_int_column", &pyEventFrameGen::add_int_column)
      .def("add_int_columns", &pyEventFrameGen::add_int_columns)
      .def("add_double_column", &pyEventFrameGen::add_double_column)
      .def("add_double_columns", &pyEventFrameGen::add_double_columns)
      .def("limit", &pyEventFrameGen::limit)
      .def("limit", [](pyEventFrameGen &s, double i) { return s.limit(i); })
      .def("norm_info", &pyEventFrameGen::norm_info)
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
