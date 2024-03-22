#pragma once

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/EventFrameGen.h"

#include "nuis/python/pyNUISANCE.h"

#include "nuis/python/pyEventInput.h"

#include "pybind11/eigen.h"
#include "pybind11/functional.h"

struct pyEventFrameGen {
  pyEventFrameGen(pyNormalizedEventSource ev, size_t bsize);

  pyEventFrameGen filter(nuis::EventFrameGen::FilterFunc filt);

  pyEventFrameGen
  add_double_columns(std::vector<std::string> const &col_names,
                     nuis::EventFrameGen::ProjectionsFunc<double> proj);

  pyEventFrameGen add_double_column(std::string const &col_name,
                                    nuis::EventFrameGen::ProjectionFunc<double> proj);

  pyEventFrameGen add_int_columns(std::vector<std::string> const &col_names,
                                  nuis::EventFrameGen::ProjectionsFunc<int> proj);

  pyEventFrameGen add_int_column(std::string const &col_name,
                                 nuis::EventFrameGen::ProjectionFunc<int> proj);

  pyEventFrameGen limit(size_t nmax);

  pyEventFrameGen progress(size_t nmax);

  nuis::EventFrame first();
  nuis::EventFrame next();
  nuis::EventFrame all();

#ifdef NUIS_ARROW_ENABLED
  pybind11::object firstArrow();
  pybind11::object nextArrow();
#endif

  std::shared_ptr<nuis::EventFrameGen> gen;
};

void pyEventFrameInit(pybind11::module &m);
