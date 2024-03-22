#pragma once

#include "nuis/histframe/HistFrame.h"

#include "nuis/eventframe/EventFrameGen.h"

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> const &proj_to_axes);
HistFrame Project(HistFrame const &hf, size_t proj_to_axis);
BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes);
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis);

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesBase const &);

void fill_from_EventFrame(
    HistFrame &hf, EventFrame &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"cv weight"});

void fill_from_EventFrame(
    HistFrame &hf, EventFrame &ef, std::string const &projection_column_name,
    std::vector<std::string> const &weight_column_names = {"cv weight"}) {
  fill_from_EventFrame(hf, ef, std::vector<std::string>{projection_column_name},
                       weight_column_names);
}

void fill_from_EventFrameGen(
    HistFrame &hf, EventFrameGen &efg,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"cv weight"});

void fill_from_EventFrameGen(
    HistFrame &hf, EventFrameGen &efg, std::string const &projection_column_name,
    std::vector<std::string> const &weight_column_names = {"cv weight"}) {
  fill_from_EventFrameGen(hf, efg,
                          std::vector<std::string>{projection_column_name},
                          weight_column_names);
}

#ifdef NUIS_ARROW_ENABLED
void fill_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> &rb,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"cv weight"});
void fill_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> &rb,
    std::string const &projection_column_name,
    std::vector<std::string> const &weight_column_names = {"cv weight"}) {
  fill_from_RecordBatch(hf, rb,
                        std::vector<std::string>{projection_column_name},
                        weight_column_names);
}
#endif

} // namespace nuis