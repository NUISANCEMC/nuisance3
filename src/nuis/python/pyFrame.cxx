#include "nuis/Frame/Frame.h"
#include "nuis/Frame/FrameGen.h"

#include "nuis/python/pyEventInput.h"
#include "nuis/python/pyYAML.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

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

  pyFrameGen add_columns(std::vector<std::string> const &col_names,
                         FrameGen::ProjectionsFunc proj) {
    *gen = gen->add_columns(col_names, proj);
    return *this;
  }

  pyFrameGen add_column(std::string const &col_name,
                        FrameGen::ProjectionFunc proj) {
    *gen = gen->add_column(col_name, proj);
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

  Frame first() { return gen->first(); }
  Frame next() { return gen->next(); }
  Frame all() { return gen->all(); }

  std::shared_ptr<FrameGen> gen;
};

Eigen::ArrayXd frame_gettattr(Frame &s, std::string const &column) {
  return s.col(column);
}

void frame_settattr(Frame &s, std::string const &column, Eigen::ArrayXd &data) {
  auto cid = s.find_column_index(column);
  if (cid != Frame::npos) {
    s.table.col(cid) = data;
  }
}

void pyFrameInit(py::module &m) {

  py::class_<Frame>(m, "Frame")
      .def(py::init<>())
      .def_readwrite("table", &Frame::table,
                     py::return_value_policy::reference_internal)
      .def_readwrite("column_names", &Frame::column_names,
                     py::return_value_policy::reference_internal)
      .def("fatx", [](Frame &s) { return s.norm_info.fatx; })
      .def("sumw", [](Frame &s) { return s.norm_info.sumweights; })
      .def("nevents", [](Frame &s) { return s.norm_info.nevents; })
      // Pandas style data access
      .def("__getattr__", &frame_gettattr)
      .def("__setattr__", &frame_settattr)
      .def("__getitem__", &frame_gettattr)
      .def("__setitem__", &frame_settattr)
      .def("__str__", [](Frame const &s) {
        std::stringstream ss("");
        ss << FramePrinter(s);
        return ss.str();
      });

  py::class_<pyFrameGen>(m, "FrameGen")
      .def(py::init<pyNormalizedEventSource, size_t>(), py::arg("event_source"),
           py::arg("block_size") = 50000)
      .def("filter", &pyFrameGen::filter)
      .def("add_column", &pyFrameGen::add_column)
      .def("add_columns", &pyFrameGen::add_columns)
      .def("limit", &pyFrameGen::limit)
      .def("progress", &pyFrameGen::progress, py::arg("every") = 100000)
      .def("first", &pyFrameGen::first)
      .def("next", &pyFrameGen::next)
      .def("all", &pyFrameGen::all);
}
