#pragma once

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/column_types.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/log.txx"

#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/bundled/ranges.h"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include <string>
#include <variant>
#include <vector>

namespace nuis {

namespace detail {

// this is an ugly do-it-all object but it enables a nice interface without a
// load of unneccessary inheritance
struct FrameFillOp {
  enum OpType {
    kNoCVWeight,
    kWeight,
    kConditional,
    kFillColumn,
    kCategorize,
    kProcID,
    kWeightedMap
  };

  FrameFillOp(OpType operation) : op(operation), doCVWeight(true) {}

  OpType op;

  std::variant<std::string, int> fill_column_id;
  std::string conditional_column_name;
  std::vector<std::string> weight_columns;
  std::vector<std::string> weighted_columns;
  std::string categorize_by_column;
  std::vector<std::string> category_labels;
  bool doCVWeight;
};

template <typename EFT> struct FrameFillPlan {};

template <> struct FrameFillPlan<EventFrame> {

  std::vector<EventFrame::column_t> proj_cols;

  std::vector<EventFrame::column_t> weight_cols;

  EventFrame::column_t conditional_col = EventFrame::npos;

  HistFrame::column_t default_fill_col = HistFrame::npos;

  EventFrame::column_t categorize_by_col = EventFrame::npos;
  std::vector<HistFrame::column_t> category_cols;

  EventFrame::column_t ProcID_col = EventFrame::npos;

  std::vector<std::pair<EventFrame::column_t, HistFrame::column_t>>
      weight_map_cols;
};

#ifdef NUIS_ARROW_ENABLED
template <> struct FrameFillPlan<std::shared_ptr<arrow::RecordBatch>> {

  std::vector<std::function<double(int)>> proj_cols;

  std::vector<std::function<double(int)>> weight_cols;

  std::function<int(int)> conditional_col;

  HistFrame::column_t default_fill_col = HistFrame::npos;

  std::function<int(int)> categorize_by_col;
  std::vector<HistFrame::column_t> category_cols;

  std::function<int(int)> ProcID_col;

  std::vector<std::pair<std::function<double(int)>, HistFrame::column_t>>
      weight_map_cols;
};
#endif

template <typename EFT, typename COLTYPE>
inline bool is_valid_col(COLTYPE const &col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return col != EventFrame::npos;
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return bool(col);
  }
#endif
}

template <typename EFT> inline std::string colinfo(EFT const &ef, int col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return (col == EventFrame::npos)
               ? "INVALIDCOLUMN"
               : fmt::format("{}<double>", ef.column_names[col]);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    throw std::runtime_error("unimplemented for arrow");
  }
#endif
  else if constexpr (std::is_same_v<EFT, HistFrame>) {
    return (col == HistFrame::npos)
               ? "INVALIDCOLUMN"
               : fmt::format("{}", ef.column_info[col].name);
  }
}

template <typename EFT, typename ROWTYPE, typename COLTYPE>
inline auto get_entry(EFT const &ef, ROWTYPE const &row, COLTYPE const &col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return ef.table(row, col);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return col(row);
  }
#endif
}

template <typename EFT>
void fill_loop(HistFrame &hf, EFT const &ef,
               FrameFillPlan<EFT> const &the_plan) {

  std::vector<int> proc_id_list;
  std::vector<HistFrame::column_t> proc_id_columns;

  if (is_valid_col<EFT>(the_plan.ProcID_col)) {

    // fill the dictionary with existing column names so that repeated calls
    // with EventFrame batches don't result in repeated columns
    for (HistFrame::column_t col_it = 0; col_it < hf.column_info.size();
         ++col_it) {

      if (hf.column_info[col_it].name.find("ProcId:") == 0) {

        try {
          auto proc_id = std::stoi(hf.column_info[col_it].name.substr(7));
          proc_id_list.push_back(proc_id);
          proc_id_columns.push_back(col_it);
        } catch (std::invalid_argument const &ia) {
          throw InvalidColumnName()
              << "[fill(EventFrame)]: Encountered "
                 "HistFrame column named "
              << hf.column_info[col_it].name
              << " but failed to cast the part after the colon to an integer.\n"
              << ia.what();
        }
      }
    }
  }

  std::vector<double> projs(the_plan.proj_cols.size(), 0);

  int num_rows = 0;
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    num_rows = ef.table.rows();
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    num_rows = ef->num_rows();
  }
#endif

  NUIS_LOGGER_TRACE("HistFrame", "fill_loop:");
  for (int row = 0; row < num_rows; ++row) {
    NUIS_LOGGER_TRACE("HistFrame", "  row: {}", row);
    if (is_valid_col<EFT>(the_plan.conditional_col)) {
      NUIS_LOGGER_TRACE("HistFrame", "    conditional: \"{}\" = {}",
                        colinfo(ef, the_plan.conditional_col),
                        get_entry(ef, row, the_plan.conditional_col));
      if (get_entry(ef, row, the_plan.conditional_col) == 0) {
        continue;
      }
    }

    double weight = 1;
    NUIS_LOGGER_TRACE("HistFrame", "    weights:");
    for (auto const &wid : the_plan.weight_cols) {
      NUIS_LOGGER_TRACE("HistFrame", "     \"{}\" = {}", colinfo(ef, wid),
                        get_entry(ef, row, wid));
      weight *= get_entry(ef, row, wid);
    }

    NUIS_LOGGER_TRACE("HistFrame", "    projections:");

    for (size_t pi = 0; pi < the_plan.proj_cols.size(); ++pi) {
      NUIS_LOGGER_TRACE("HistFrame", "      \"{}\" = {}",
                        colinfo(ef, the_plan.proj_cols[pi]),
                        get_entry(ef, row, the_plan.proj_cols[pi]));
      projs[pi] = get_entry(ef, row, the_plan.proj_cols[pi]);
    }

    auto bin = hf.find_bin(projs);
    NUIS_LOGGER_TRACE("HistFrame", "    --> bin = {}", bin);

    if (the_plan.default_fill_col != HistFrame::npos) {
      NUIS_LOGGER_TRACE("HistFrame", "    fill hist column: {}",
                        colinfo(hf, the_plan.default_fill_col));
      hf.fill_bin(bin, weight, the_plan.default_fill_col);
    }

    if (is_valid_col<EFT>(the_plan.categorize_by_col)) {
      HistFrame::column_t col = get_entry(ef, row, the_plan.categorize_by_col);
      if (the_plan.category_cols.size()) {
        if (col < the_plan.category_cols.size()) {
          hf.fill_bin(bin, weight, the_plan.category_cols[col]);
        }
      } else if (col != the_plan.default_fill_col) {
        if (col < hf.column_info.size()) {
          hf.fill_bin(bin, weight, col);
        }
      }
    }

    if (is_valid_col<EFT>(the_plan.ProcID_col)) {
      int procid = get_entry(ef, row, the_plan.ProcID_col);
      auto pid_it = std::find(proc_id_list.begin(), proc_id_list.end(), procid);

      if (pid_it == proc_id_list.end()) {
        proc_id_list.push_back(procid);
        proc_id_columns.push_back(
            hf.add_column(fmt::format("ProcId:{}", procid)));
        pid_it = std::find(proc_id_list.begin(), proc_id_list.end(), procid);
      }

      HistFrame::column_t col =
          proc_id_columns[(pid_it - proc_id_list.begin())];

      hf.fill_bin(bin, weight, col);
    }

    for (auto const &cmap : the_plan.weight_map_cols) {
      hf.fill_bin(bin, weight * get_entry(ef, row, cmap.first), cmap.second);
    }
  }
}

template <typename CAST_TO, typename EFT>
inline auto require_column_index(EFT const &ef, std::string const &cn) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return ef.require_column_index(cn);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return get_col_cast_to<CAST_TO>(ef, cn);
  }
#endif
}

template <typename EFT>
void fill(HistFrame &hf, EFT const &ef,
          std::vector<std::string> const &projection_columns,
          std::vector<FrameFillOp> operations) {

  FrameFillPlan<EFT> the_plan;
  bool weight_by_CV = true;

  // parse operations
  NUIS_LOGGER_DEBUG("HistFrame", "fill plan:");
  for (auto const &op : operations) {

    if (op.op == FrameFillOp::kNoCVWeight) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- Do not apply CV weight");
      weight_by_CV = false;
    }

    if (op.op == FrameFillOp::kWeight) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- adding weighing columns:");
      for (auto const &weight_col_name : op.weight_columns) {
        NUIS_LOGGER_DEBUG("HistFrame", "    * {}", weight_col_name);
        the_plan.weight_cols.push_back(
            require_column_index<double>(ef, weight_col_name));
      }
    }

    if (op.op == FrameFillOp::kConditional) {

      if (is_valid_col<EFT>(the_plan.conditional_col)) {
        throw InvalidOperation()
            << "fill passed more than one fill_if operations.";
      }

      the_plan.conditional_col =
          require_column_index<int>(ef, op.conditional_column_name);

      NUIS_LOGGER_DEBUG("HistFrame", "  -- conditional column. Fill if {} != 0",
                        colinfo(ef, the_plan.conditional_col));
    }

    if (op.op == FrameFillOp::kFillColumn) {

      if (the_plan.default_fill_col != HistFrame::npos) {
        throw InvalidOperation()
            << "fill passed more than one fill_column operations.";
      }

      if (std::holds_alternative<int>(op.fill_column_id)) {
        the_plan.default_fill_col = std::get<int>(op.fill_column_id);
        if (the_plan.default_fill_col >= hf.column_info.size()) {
          throw InvalidColumnAccess()
              << fmt::format("fill_column(int col={}) passed but histogram "
                             "only has {} existing columns, if you would like "
                             "to automatically make a output column if one "
                             "doesn't exist, use fill_column(std::string).",
                             the_plan.default_fill_col, hf.column_info.size());
        }
      } else {
        auto const &cname = std::get<std::string>(op.fill_column_id);
        the_plan.default_fill_col = hf.find_column_index(cname);
        if (the_plan.default_fill_col == HistFrame::npos) {
          the_plan.default_fill_col = hf.add_column(cname);
        }
      }

      NUIS_LOGGER_DEBUG("HistFrame", "  -- Fill column: {}",
                        colinfo(hf, the_plan.default_fill_col));
    }

    if (op.op == FrameFillOp::kCategorize) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- categorize");
      if (op.category_labels.size()) {
        for (auto const &cat_label : op.category_labels) {
          the_plan.category_cols.push_back(hf.find_column_index(cat_label));
          if (the_plan.category_cols.back() == HistFrame::npos) {
            the_plan.category_cols.back() = hf.add_column(cat_label);
          }
        }
      }

      the_plan.categorize_by_col =
          require_column_index<int>(ef, op.categorize_by_column);
    }

    if (op.op == FrameFillOp::kProcID) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- split by procid");
      the_plan.ProcID_col = require_column_index<int>(ef, "process.id");
    }

    if (op.op == FrameFillOp::kWeightedMap) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- weighted column map");
      for (auto const &weight_col_name : op.weighted_columns) {
        the_plan.weight_map_cols.emplace_back(
            require_column_index<double>(ef, weight_col_name),
            hf.find_column_index(weight_col_name));

        if (the_plan.weight_map_cols.back().second == HistFrame::npos) {
          the_plan.weight_map_cols.back().second =
              hf.add_column(weight_col_name);
        }
      }
    }
  }

  if (weight_by_CV) {
    the_plan.weight_cols.push_back(
        require_column_index<double>(ef, "weight.cv"));
  }

  for (auto const &proj_col_name : projection_columns) {
    the_plan.proj_cols.push_back(
        require_column_index<double>(ef, proj_col_name));
    nuis_named_log("HistFrame")::log_trace("[fill(EventFrame)]: proj({})",
                                           proj_col_name);
  }

  fill_loop(hf, ef, the_plan);
}

#ifdef NUIS_ARROW_ENABLED
template <>
void fill(HistFrame &hf, std::shared_ptr<arrow::Table> const &at,
          std::vector<std::string> const &projection_columns,
          std::vector<FrameFillOp> operations) {
  for (auto rb : arrow::TableBatchReader(at)) {
    fill(hf, rb.ValueOrDie(), projection_columns, operations);
  }
}
#endif

} // namespace detail

inline detail::FrameFillOp fill_column(std::string const &cname) {
  detail::FrameFillOp op(detail::FrameFillOp::kFillColumn);
  op.fill_column_id = cname;
  return op;
}

inline detail::FrameFillOp fill_column(int col_num) {
  detail::FrameFillOp op(detail::FrameFillOp::kFillColumn);
  op.fill_column_id = col_num;
  return op;
}

inline detail::FrameFillOp fill_if(std::string const &cname) {
  detail::FrameFillOp op(detail::FrameFillOp::kConditional);
  op.conditional_column_name = cname;
  return op;
}

inline detail::FrameFillOp weight_by(std::vector<std::string> const &cnames) {
  detail::FrameFillOp op(detail::FrameFillOp::kWeight);
  op.weight_columns = cnames;
  return op;
}

inline detail::FrameFillOp weight_by(std::string const &cname) {
  return weight_by(std::vector<std::string>{
      cname,
  });
}

inline detail::FrameFillOp
weighted_column_map(std::vector<std::string> const &cnames) {
  detail::FrameFillOp op(detail::FrameFillOp::kWeightedMap);
  op.weighted_columns = cnames;
  return op;
}

inline detail::FrameFillOp weighted_column_map(std::string const &cname) {
  return weighted_column_map(std::vector<std::string>{
      cname,
  });
}

inline detail::FrameFillOp
categorize_by(std::string const &cname,
              std::vector<std::string> const &category_labels = {}) {
  detail::FrameFillOp op(detail::FrameFillOp::kCategorize);
  op.categorize_by_column = cname;
  op.category_labels = category_labels;
  return op;
}

inline detail::FrameFillOp split_by_ProcID() {
  detail::FrameFillOp op(detail::FrameFillOp::kProcID);
  return op;
}

inline detail::FrameFillOp no_CV_weight() {
  detail::FrameFillOp op(detail::FrameFillOp::kNoCVWeight);
  return op;
}

template <typename EFT, typename... OPs>
void fill(HistFrame &hf, EFT const &ef,
          std::vector<std::string> const &projection_columns, OPs... op_pack) {
  detail::fill(hf, ef, projection_columns,
               std::vector<detail::FrameFillOp>{op_pack...});
}

// if no options are passed, just fill column 0
template <typename EFT, typename... OPs>
void fill(HistFrame &hf, EFT const &ef,
          std::vector<std::string> const &projection_columns) {
  fill(hf, ef, projection_columns, fill_column(0));
}

} // namespace nuis
