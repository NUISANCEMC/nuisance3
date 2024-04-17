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
  add_bool_columns(std::vector<std::string> const &col_names,
                   nuis::EventFrameGen::ProjectionsFunc<bool> proj);
  pyEventFrameGen
  add_bool_column(std::string const &col_name,
                  nuis::EventFrameGen::ProjectionFunc<bool> proj);

  pyEventFrameGen
  add_int_columns(std::vector<std::string> const &col_names,
                  nuis::EventFrameGen::ProjectionsFunc<int> proj);
  pyEventFrameGen add_int_column(std::string const &col_name,
                                 nuis::EventFrameGen::ProjectionFunc<int> proj);

  pyEventFrameGen
  add_uint_columns(std::vector<std::string> const &col_names,
                   nuis::EventFrameGen::ProjectionsFunc<uint> proj);
  pyEventFrameGen
  add_uint_column(std::string const &col_name,
                  nuis::EventFrameGen::ProjectionFunc<uint> proj);

  pyEventFrameGen
  add_int16_columns(std::vector<std::string> const &col_names,
                    nuis::EventFrameGen::ProjectionsFunc<int16_t> proj);
  pyEventFrameGen
  add_int16_column(std::string const &col_name,
                   nuis::EventFrameGen::ProjectionFunc<int16_t> proj);

  pyEventFrameGen
  add_uint16_columns(std::vector<std::string> const &col_names,
                     nuis::EventFrameGen::ProjectionsFunc<uint16_t> proj);
  pyEventFrameGen
  add_uint16_column(std::string const &col_name,
                    nuis::EventFrameGen::ProjectionFunc<uint16_t> proj);

  pyEventFrameGen
  add_float_columns(std::vector<std::string> const &col_names,
                    nuis::EventFrameGen::ProjectionsFunc<float> proj);

  pyEventFrameGen
  add_float_column(std::string const &col_name,
                   nuis::EventFrameGen::ProjectionFunc<float> proj);

  pyEventFrameGen
  add_double_columns(std::vector<std::string> const &col_names,
                     nuis::EventFrameGen::ProjectionsFunc<double> proj);

  pyEventFrameGen
  add_double_column(std::string const &col_name,
                    nuis::EventFrameGen::ProjectionFunc<double> proj);

  pyEventFrameGen limit(size_t nmax);

  pyEventFrameGen progress(size_t nmax);

  nuis::EventFrame first(size_t nchunk);
  nuis::EventFrame next(size_t nchunk);
  nuis::EventFrame all();

#ifdef NUIS_ARROW_ENABLED
  pybind11::object firstArrow(size_t nchunk);
  pybind11::object nextArrow(size_t nchunk);
#endif

  nuis::NormInfo norm_info() const;

  std::shared_ptr<nuis::EventFrameGen> gen;
};

void pyEventFrameInit(pybind11::module &m);
