#include "nuis/Frame/Frame.h"
#include "nuis/Frame/FrameGen.h"

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

struct pyFrameGen {
  pyFrameGen(pyNormalizedEventSource ev, size_t bsize) {
    gen = std::make_shared<FrameGen>(ev.evs, bsize);
  }

  pyFrameGen filter(FrameGen::FilterFunc filt) {
    *gen = gen->filter(filt);
    return *this;
  }

  pyFrameGen add_double_columns(std::vector<std::string> const &col_names,
                                FrameGen::ProjectionsFunc<double> proj) {
    *gen = gen->add_typed_columns<double>(col_names, proj);
    return *this;
  }

  pyFrameGen add_double_column(std::string const &col_name,
                               FrameGen::ProjectionFunc<double> proj) {
    *gen = gen->add_typed_column<double>(col_name, proj);
    return *this;
  }

  pyFrameGen add_int_columns(std::vector<std::string> const &col_names,
                             FrameGen::ProjectionsFunc<int> proj) {
    *gen = gen->add_typed_columns<int>(col_names, proj);
    return *this;
  }

  pyFrameGen add_int_column(std::string const &col_name,
                            FrameGen::ProjectionFunc<int> proj) {
    *gen = gen->add_typed_column<int>(col_name, proj);
    return *this;
  }

  pyFrameGen limit(size_t nmax) {
    *gen = gen->limit(nmax);
    return *this;
  }

  pyFrameGen progress(size_t nmax) {
    *gen = gen->progress(nmax);
    return *this;
  }

  auto first() { return gen->first(); }
  auto next() { return gen->next(); }
  auto all() { return gen->all(); }

#ifdef NUIS_ARROW_ENABLED
  auto firstArrow() {
    return py::reinterpret_steal<py::object>(
        arrow::py::wrap_batch(gen->firstArrow()));
  }
  auto nextArrow() {
    return py::reinterpret_steal<py::object>(
        arrow::py::wrap_batch(gen->nextArrow()));
  }
#endif

  std::shared_ptr<FrameGen> gen;
};

Eigen::ArrayXdRef frame_gettattr(Frame &s, std::string const &column) {
  return s.col(column);
}

void frame_settattr(Frame &s, std::string const &column,
                    Eigen::ArrayXdRef const data) {
  auto cid = s.find_column_index(column);
  if (cid != Frame::npos) {
    s.table.col(cid) = data;
  }
}

void pyFrameInit(py::module &m) {

#ifdef NUIS_ARROW_ENABLED
  arrow::py::import_pyarrow();
  m.add_object("pa", py::module::import("pyarrow"));
#endif

  py::class_<Frame>(m, "Frame")
      .def(py::init<>())
      .def_readwrite("table", &Frame::table,
                     py::return_value_policy::reference_internal)
      .def_readwrite("column_names", &Frame::column_names,
                     py::return_value_policy::reference_internal)
      .def("fatx", [](Frame const &s) { return s.norm_info.fatx; })
      .def("sumw", [](Frame const &s) { return s.norm_info.sumweights; })
      .def("nevents", [](Frame const &s) { return s.norm_info.nevents; })
      .def("rows", [](Frame const &s) { return s.table.rows(); })
      .def("find_column_index", &Frame::find_column_index)
      .def_readonly_static("npos", &Frame::npos)
      .def_readonly_static("missing_datum", &kMissingDatum<double>)
      .def("__getattr__", &frame_gettattr)
      .def("__setattr__", &frame_settattr)
      .def("__getitem__", &frame_gettattr)
      .def("__setitem__", &frame_settattr)
      .def("__repr__", &str_via_ss<Frame>);

  py::class_<pyFrameGen>(m, "FrameGen")
      .def(py::init<pyNormalizedEventSource, size_t>(), py::arg("event_source"),
           py::arg("block_size") = 50000)
      .def("filter", &pyFrameGen::filter)
#ifdef NUIS_ARROW_ENABLED
      .def_static("has_arrow_support", []() { return true; })
#else
      .def_static("has_arrow_support", []() { return false; })
#endif
      .def("add_column", &pyFrameGen::add_double_column)
      .def("add_columns", &pyFrameGen::add_double_columns)
      .def("add_int_column", &pyFrameGen::add_int_column)
      .def("add_int_columns", &pyFrameGen::add_int_columns)
      .def("add_double_column", &pyFrameGen::add_double_column)
      .def("add_double_columns", &pyFrameGen::add_double_columns)
      .def("limit", &pyFrameGen::limit)
      .def("progress", &pyFrameGen::progress, py::arg("every") = 100000)
      .def("first", &pyFrameGen::first)
      .def("next", &pyFrameGen::next)
#ifdef NUIS_ARROW_ENABLED
      .def("firstArrow", &pyFrameGen::firstArrow)
      .def("nextArrow", &pyFrameGen::nextArrow)
#endif
      .def("all", &pyFrameGen::all);
}
