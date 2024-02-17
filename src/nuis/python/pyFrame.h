#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/stl_bind.h"

#include "yaml-cpp/yaml.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

#include "spdlog/spdlog.h"

#include "nuis/frame/FrameGen.h"
#include "nuis/frame/Frame.h"

#include "nuis/python/pyEventInput.h"


namespace py = pybind11;

namespace nuis {
struct pyFrameGen {
  using FilterFunc = std::function<bool(HepMC3::GenEvent const &)>;
  using ProjectionFunc = std::function<double(HepMC3::GenEvent const &)>;
  using ProjectionsFunc =
      std::function<std::vector<double>(HepMC3::GenEvent const &)>;

  pyFrameGen(pyNormalizedEventSource ev, size_t bsize) {
    gen = new FrameGen(ev.evs, bsize);
  }

  pyFrameGen Filter(FilterFunc filt) {
    *gen = gen->Filter(filt);
    return *this;
  }

  pyFrameGen AddColumns(std::vector<std::string> col_names, ProjectionsFunc proj) {
    *gen = gen->AddColumns(col_names, proj);
    return *this;
  }

  pyFrameGen AddColumn(std::string col_name, ProjectionFunc proj) {
    *gen = gen->AddColumn(col_name, proj);
    return *this;
  }

  pyFrameGen Limit(size_t nmax){
    *gen = gen->Limit(nmax);
    return *this;
  }

  Frame Evaluate(){
    return gen->Evaluate();
  }

  FrameGen* gen;
};
}



void init_frame(py::module &m) {

    py::class_<nuis::Frame>(m, "Frame")
        .def(py::init<>())
        .def_readwrite("Table",
            &nuis::Frame::Table, py::return_value_policy::reference_internal)
        .def_readwrite("ColumnNames",
            &nuis::Frame::ColumnNames,
            py::return_value_policy::reference_internal)
        .def("fatx", [](nuis::Frame &s){ return s.norm_info.fatx; })
        .def("sumw", [](nuis::Frame &s){ return s.norm_info.sumweights; })
        .def("nevents", [](nuis::Frame &s){ return s.norm_info.nevents; });

    py::class_<nuis::pyFrameGen>(m, "FrameGen")
        .def(py::init<pyNormalizedEventSource, size_t>())
        .def("Filter", &nuis::pyFrameGen::Filter)
        .def("AddColumn", &nuis::pyFrameGen::AddColumn)
        .def("Limit", &nuis::pyFrameGen::Limit)
        .def("Evaluate", &nuis::pyFrameGen::Evaluate);

}


