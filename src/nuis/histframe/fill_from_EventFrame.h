#pragma once

#include "nuis/histframe/HistFrame.h"

#include "nuis/eventframe/EventFrameGen.h"

namespace nuis {

void fill_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_new_column_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

void fill_new_column_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_name,
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
void fill_new_column_from_Arrow(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

template <typename ArrTabular = arrow::RecordBatch>
void fill_new_column_from_Arrow_if(
    HistFrame &hf, std::shared_ptr<ArrTabular> const &tab,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_name,
    std::vector<std::string> const &weight_column_names = {"weight.cv"});

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