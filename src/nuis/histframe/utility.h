#pragma once

#include "nuis/histframe/HistFrame.h"

#include "nuis/eventframe/EventFrameGen.h"

namespace nuis {

HistFrame Project(HistFrame const &hf, std::vector<size_t> const &proj_to_axes,
                  bool result_has_binning = true);
HistFrame Project(HistFrame const &hf, size_t proj_to_axis,
                  bool result_has_binning = true);
BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes,
                     bool result_has_binning = true);
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis,
                     bool result_has_binning = true);

HistFrame Slice(HistFrame const &hf, size_t ax,
                std::array<double, 2> slice_range,
                bool exclude_range_end_bin = false,
                bool result_has_binning = true);
HistFrame Slice(HistFrame const &hf, size_t ax, double slice_val,
                bool result_has_binning = true);
BinnedValues Slice(BinnedValues const &hf, size_t ax,
                   std::array<double, 2> slice_range,
                   bool exclude_range_end_bin = false,
                   bool result_has_binning = true);
BinnedValues Slice(BinnedValues const &hf, size_t ax, double slice_val,
                   bool result_has_binning = true);

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesBase const &);

void fill_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_weighted_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_weighted_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_procid_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_procid_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_from_EventFrameGen(
    HistFrame &hf, EventFrameGen &efg,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

#ifdef NUIS_ARROW_ENABLED
template <typename ArrTabular = arrow::RecordBatch>
void fill_from_Arrow(HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
                     std::vector<std::string> const &projection_column_names,
                     std::vector<std::string> const &weight_column_names = {
                         "weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_from_Arrow_if(HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
                        std::string const &conditional_column_name,
                        std::vector<std::string> const &projection_column_names,
                        std::vector<std::string> const &weight_column_names = {
                            "weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_columns_from_Arrow(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_columns_from_Arrow_if(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_weighted_columns_from_Arrow(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_weighted_columns_from_Arrow_if(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_procid_columns_from_Arrow(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_procid_columns_from_Arrow_if(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

#endif

} // namespace nuis