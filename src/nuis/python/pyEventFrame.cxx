#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/EventFrameGen.h"

#include "nuis/python/pyNUISANCE.h"

#include "nuis/python/pyEventInput.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

#ifdef NUIS_ARROW_ENABLED
// header that
#include "arrow/python/pyarrow.h"
#endif

namespace py = pybind11;

using namespace nuis;

struct pyEventFrameGen {
  pyEventFrameGen(pyNormalizedEventSource ev, size_t bsize) {
    gen = std::make_shared<EventFrameGen>(ev.evs, bsize);
  }

  pyEventFrameGen filter(EventFrameGen::FilterFunc filt) {
    *gen = gen->filter(filt);
    return *this;
  }

  pyEventFrameGen
  add_double_columns(std::vector<std::string> const &col_names,
                     EventFrameGen::ProjectionsFunc<double> proj) {
    *gen = gen->add_typed_columns<double>(col_names, proj);
    return *this;
  }

  pyEventFrameGen
  add_double_column(std::string const &col_name,
                    EventFrameGen::ProjectionFunc<double> proj) {
    *gen = gen->add_typed_column<double>(col_name, proj);
    return *this;
  }

  pyEventFrameGen add_int_columns(std::vector<std::string> const &col_names,
                                  EventFrameGen::ProjectionsFunc<int> proj) {
    *gen = gen->add_typed_columns<int>(col_names, proj);
    return *this;
  }

  pyEventFrameGen add_int_column(std::string const &col_name,
                                 EventFrameGen::ProjectionFunc<int> proj) {
    *gen = gen->add_typed_column<int>(col_name, proj);
    return *this;
  }

  pyEventFrameGen limit(size_t nmax) {
    *gen = gen->limit(nmax);
    return *this;
  }

  pyEventFrameGen progress(size_t nmax) {
    *gen = gen->progress(nmax);
    return *this;
  }

  auto first() { return gen->first(); }
  auto next() { return gen->next(); }
  auto all() { return gen->all(); }

#ifdef NUIS_ARROW_ENABLED
  py::object firstArrow() {
    auto rb = gen->firstArrow();
    if (rb) {
      return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
    }
    return py::none();
  }
  py::object nextArrow() {
    auto rb = gen->nextArrow();
    if (rb) {
      return py::reinterpret_steal<py::object>(arrow::py::wrap_batch(rb));
    }
    return py::none();
  }
#endif

  std::shared_ptr<EventFrameGen> gen;
};

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
      .def("fatx", [](EventFrame const &s) { return s.norm_info.fatx; })
      .def("sumw", [](EventFrame const &s) { return s.norm_info.sumweights; })
      .def("nevents", [](EventFrame const &s) { return s.norm_info.nevents; })
      .def("rows", [](EventFrame const &s) { return s.table.rows(); })
      .def("find_column_index", &EventFrame::find_column_index)
      .def_readonly_static("npos", &EventFrame::npos)
      .def_readonly_static("missing_datum", &kMissingDatum<double>)
      .def("__getattr__", &frame_gettattr)
      .def("__setattr__", &frame_settattr)
      .def("__getitem__", &frame_gettattr)
      .def("__setitem__", &frame_settattr)
      .def("__repr__", &str_via_ss<EventFrame>);

  py::class_<pyEventFrameGen>(m, "EventFrameGen")
      .def(py::init<pyNormalizedEventSource, size_t>(), py::arg("event_source"),
           py::arg("block_size") = 50000)
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
      .def("progress", &pyEventFrameGen::progress, py::arg("every") = 100000)
      .def("first", &pyEventFrameGen::first)
      .def("next", &pyEventFrameGen::next)
#ifdef NUIS_ARROW_ENABLED
      .def("firstArrow", &pyEventFrameGen::firstArrow)
      .def("nextArrow", &pyEventFrameGen::nextArrow)
#endif
      .def("all", &pyEventFrameGen::all);
}
