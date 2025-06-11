#pragma once

#include "nuis/eventframe/EventFrame.h"
#include "nuis/eventframe/column_types.h"
#include "nuis/eventframe/utility.h"

#include "nuis/histframe/HistFrame.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/log.txx"

#include "fmt/core.h"
#include "fmt/ranges.h"

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include <string>
#include <variant>
#include <vector>

namespace nuis {

namespace detail {

template <typename EFT, typename T> inline std::string colinfo(EFT const &ef, T col) {
  if constexpr (std::is_same_v<EFT, EventFrame>) {
    return (col == EventFrame::npos)
               ? "INVALIDCOLUMN"
               : fmt::format("{}<double>", ef.column_names[col]);
  }
#ifdef NUIS_ARROW_ENABLED
  else if constexpr (std::is_same_v<EFT, std::shared_ptr<arrow::RecordBatch>>) {
    return "ARROW-PLACEHOLDER-COLUMN";
  }
#endif
  else if constexpr (std::is_same_v<EFT, HistFrame>) {
    return (col == HistFrame::npos)
               ? "INVALIDCOLUMN"
               : fmt::format("{}", ef.column_info[col].name);
  }
}

// this is an ugly do-it-all object but it enables a nice interface without a
// load of unneccessary inheritance
struct FrameFillOp {
  enum OpType {
    kNoCVWeight,
    kWeight,
    kWeightArray,
    kConditional,
    kFillColumn,
    kCategorize,
    kWeightedMap,
    kBinIdColumn,
    kBinIdArray
  };

  FrameFillOp(OpType operation) : op(operation), doCVWeight(true) {}

  OpType op;

  std::variant<std::string, int> fill_column_id;
  std::string conditional_column_name;
  int conditional_value = std::numeric_limits<int>::max();
  std::vector<std::string> weight_columns;
  std::vector<std::string> weighted_columns;
  std::string categorize_by_column;
  std::vector<std::string> category_labels;
  bool doCVWeight;
  Eigen::ArrayXd weights;
  std::string BinId_column;
  Eigen::ArrayXi BinIds;
};

template <typename EFT> struct FrameFillPlan {};

template <> struct FrameFillPlan<EventFrame> {

  std::vector<EventFrame::column_t> proj_cols;

  std::vector<EventFrame::column_t> weight_cols;

  EventFrame::column_t conditional_col = EventFrame::npos;
  int conditional_value = std::numeric_limits<int>::max();

  HistFrame::column_t default_fill_col = HistFrame::npos;

  EventFrame::column_t categorize_by_col = EventFrame::npos;
  std::string categorize_by_colname;
  bool categories_are_labelled;
  std::vector<HistFrame::column_t> category_cols;

  std::vector<std::pair<EventFrame::column_t, HistFrame::column_t>>
      weight_map_cols;

  Eigen::ArrayXd weights;

  HistFrame::column_t BinId_col = HistFrame::npos;
  Eigen::ArrayXi BinIds;

  bool fast;
};

#ifdef NUIS_ARROW_ENABLED
template <> struct FrameFillPlan<std::shared_ptr<arrow::RecordBatch>> {

  std::vector<std::function<double(int)>> proj_cols;

  std::vector<std::function<double(int)>> weight_cols;

  std::function<int(int)> conditional_col;
  int conditional_value = std::numeric_limits<int>::max();

  HistFrame::column_t default_fill_col = HistFrame::npos;

  std::function<int(int)> categorize_by_col;
  std::string categorize_by_colname;
  bool categories_are_labelled;
  std::vector<HistFrame::column_t> category_cols;

  std::vector<std::pair<std::function<double(int)>, HistFrame::column_t>>
      weight_map_cols;

  Eigen::ArrayXd weights;

  std::function<int(int)> BinId_col;
  Eigen::ArrayXi BinIds;

  bool fast;
};
#endif

template <bool bin_array, typename EFT>
void fast_fill(HistFrame &hf, EFT const &ef,
               FrameFillPlan<EFT> const &the_plan) {

  int num_rows = eft::num_rows(ef);

  NUIS_LOGGER_TRACE("HistFrame", "fast_fill:");
  for (int row = 0; row < num_rows; ++row) {
    Binning::index_t bin = Binning::npos;
    if constexpr (bin_array) {
      bin = the_plan.BinIds(row);
    } else {
      bin = eft::get_entry(ef, row, the_plan.BinId_col);
    }
    hf.fill_bin(bin, eft::get_entry(ef, row, the_plan.weight_cols.front()),
                the_plan.default_fill_col);
  }
}

template <bool bin_array, typename EFT>
void fast_fill_weightarray(HistFrame &hf, EFT const &ef,
                           FrameFillPlan<EFT> const &the_plan) {

  int num_rows = eft::num_rows(ef);
  NUIS_LOGGER_TRACE("HistFrame", "fast_fill_weightarray:");
  for (int row = 0; row < num_rows; ++row) {
    Binning::index_t bin = Binning::npos;
    if constexpr (bin_array) {
      bin = the_plan.BinIds(row);
    } else {
      bin = eft::get_entry(ef, row, the_plan.BinId_col);
    }
    hf.fill_bin(bin,
                eft::get_entry(ef, row, the_plan.weight_cols.front()) *
                    the_plan.weights(row),
                the_plan.default_fill_col);
  }
}

template <bool bin_array, typename EFT>
void fast_fill_noCV(HistFrame &hf, EFT const &ef,
                    FrameFillPlan<EFT> const &the_plan) {

  int num_rows = eft::num_rows(ef);
  NUIS_LOGGER_TRACE("HistFrame", "fast_fill_noCV:");
  for (int row = 0; row < num_rows; ++row) {
    Binning::index_t bin = Binning::npos;
    if constexpr (bin_array) {
      bin = the_plan.BinIds(row);
    } else {
      bin = eft::get_entry(ef, row, the_plan.BinId_col);
    }
    hf.fill_bin(bin, 1.0, the_plan.default_fill_col);
  }
}

template <typename EFT>
void fill_loop(HistFrame &hf, EFT const &ef,
               FrameFillPlan<EFT> const &the_plan) {

  std::vector<int> unlabelled_categ_list;
  std::vector<HistFrame::column_t> unlabelled_categ_columns;

  if (eft::is_valid_col<EFT>(the_plan.categorize_by_col) &&
      !the_plan.categories_are_labelled) {

    // fill the dictionary with existing column names so that repeated calls
    // with EventFrame batches don't result in repeated columns
    for (HistFrame::column_t col_it = 0; col_it < hf.column_info.size();
         ++col_it) {

      if (hf.column_info[col_it].name.find(
              fmt::format("{}:", the_plan.categorize_by_colname)) == 0) {

        try {
          auto categ_val =
              std::stol(hf.column_info[col_it].name.substr(
                            the_plan.categorize_by_colname.size() + 1),
                        nullptr, 10);
          unlabelled_categ_list.push_back(categ_val);
          unlabelled_categ_columns.push_back(col_it);
        } catch (std::invalid_argument const &ia) {
          throw InvalidColumnName() << "[fill(EventFrame)]: Encountered "
                                       "HistFrame column named "
                                    << hf.column_info[col_it].name
                                    << " but failed to cast the part after "
                                       "the colon to an integer.\n"
                                    << ia.what();
        }
      }
    }
  }

  std::vector<double> projs(the_plan.proj_cols.size(), 0);

  int num_rows = eft::num_rows(ef);

  NUIS_LOGGER_TRACE("HistFrame", "fill_loop:");
  for (int row = 0; row < num_rows; ++row) {
    NUIS_LOGGER_TRACE("HistFrame", "  row: {}", row);
    if (eft::is_valid_col<EFT>(the_plan.conditional_col)) {
      NUIS_LOGGER_TRACE("HistFrame", "    conditional: \"{}\" = {}",
                        colinfo(ef, the_plan.conditional_col),
                        eft::get_entry(ef, row, the_plan.conditional_col));
      auto val = eft::get_entry(ef, row, the_plan.conditional_col);
      if (the_plan.conditional_value != std::numeric_limits<int>::max()) {
        if (val != the_plan.conditional_value) {
          continue;
        }
      } else {
        if (val == 0) {
          continue;
        }
      }
    }

    double weight = 1;
    NUIS_LOGGER_TRACE("HistFrame", "    weights:");
    for (auto const &wid : the_plan.weight_cols) {
      NUIS_LOGGER_TRACE("HistFrame", "     \"{}\" = {}", colinfo(ef, wid),
                        eft::get_entry(ef, row, wid));
      weight *= eft::get_entry(ef, row, wid);
    }

    if (the_plan.weights.size()) {
      weight *= the_plan.weights(row);
    }

    NUIS_LOGGER_TRACE("HistFrame", "    projections:");

    Binning::index_t bin = Binning::npos;

    if (eft::is_valid_col<EFT>(the_plan.BinId_col)) {
      bin = eft::get_entry(ef, row, the_plan.BinId_col);
    } else if (the_plan.BinIds.size()) {
      bin = the_plan.BinIds(row);
    } else {
      for (size_t pi = 0; pi < the_plan.proj_cols.size(); ++pi) {
        NUIS_LOGGER_TRACE("HistFrame", "      \"{}\" = {}",
                          colinfo(ef, the_plan.proj_cols[pi]),
                          eft::get_entry(ef, row, the_plan.proj_cols[pi]));
        projs[pi] = eft::get_entry(ef, row, the_plan.proj_cols[pi]);
      }
      bin = hf.find_bin(projs);
    }
    NUIS_LOGGER_TRACE("HistFrame", "    --> bin = {}", bin);

    if (the_plan.default_fill_col != HistFrame::npos) {
      NUIS_LOGGER_TRACE("HistFrame", "    fill hist column: {}",
                        colinfo(hf, the_plan.default_fill_col));
      hf.fill_bin(bin, weight, the_plan.default_fill_col);
    }

    if (eft::is_valid_col<EFT>(the_plan.categorize_by_col)) {
      int val = eft::get_entry(ef, row, the_plan.categorize_by_col);
      if (the_plan.categories_are_labelled) {
        if (val < int(the_plan.category_cols.size())) {
          hf.fill_bin(bin, weight, the_plan.category_cols[val]);
        }
      } else {
        auto categ_it = std::find(unlabelled_categ_list.begin(),
                                  unlabelled_categ_list.end(), val);

        if (categ_it == unlabelled_categ_list.end()) {
          unlabelled_categ_list.push_back(val);
          unlabelled_categ_columns.push_back(hf.add_column(
              fmt::format("{}:{}", the_plan.categorize_by_colname, val)));
          categ_it = std::find(unlabelled_categ_list.begin(),
                               unlabelled_categ_list.end(), val);
        }

        HistFrame::column_t col = unlabelled_categ_columns[(
            categ_it - unlabelled_categ_list.begin())];

        hf.fill_bin(bin, weight, col);
      }
    }

    for (auto const &cmap : the_plan.weight_map_cols) {
      hf.fill_bin(bin, weight * eft::get_entry(ef, row, cmap.first),
                  cmap.second);
    }
  }
}

template <typename EFT>
void fill(HistFrame &hf, EFT const &ef,
          std::vector<std::string> const &projection_columns,
          std::vector<FrameFillOp> operations) {

  FrameFillPlan<EFT> the_plan;
  the_plan.fast = true;
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
            eft::require_column_index<double>(ef, weight_col_name));
      }
      the_plan.fast = false;
    }

    if (op.op == FrameFillOp::kWeightArray) {
      NUIS_LOGGER_DEBUG("HistFrame", "  -- adding weighting array.");

      int num_rows = eft::num_rows(ef);

      if (num_rows != op.weights.size()) {
        throw InvalidOperation()
            << "WeightArray operation passed with invalid size: "
            << op.weights.size()
            << ", it must match "
               "the number of rows in the EventFrame, which has "
            << num_rows << " rows";
      }

      if (!the_plan.weights.size()) {
        the_plan.weights = Eigen::ArrayXd::Constant(num_rows, 1);
      }

      the_plan.weights *= op.weights;
    }

    if (op.op == FrameFillOp::kConditional) {

      if (eft::is_valid_col<EFT>(the_plan.conditional_col)) {
        throw InvalidOperation()
            << "fill passed more than one fill_if[_eq] operations.";
      }

      the_plan.conditional_col =
          eft::require_column_index<int>(ef, op.conditional_column_name);

      if (op.conditional_value == std::numeric_limits<int>::max()) {
        NUIS_LOGGER_DEBUG("HistFrame",
                          "  -- conditional column. Fill if {} != 0",
                          colinfo(ef, the_plan.conditional_col));
      } else {
        NUIS_LOGGER_DEBUG(
            "HistFrame", "  -- conditional column. Fill if {} == {}",
            colinfo(ef, the_plan.conditional_col), op.conditional_value);
        the_plan.conditional_value = op.conditional_value;
      }

      the_plan.fast = false;
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

      if (eft::is_valid_col<EFT>(the_plan.categorize_by_col)) {
        throw InvalidOperation() << "fill passed more than one categorize_by "
                                    "(split_by_ProcID is one) operations.";
      }

      if (the_plan.weight_map_cols.size()) {
        throw InvalidOperation()
            << "fill passed both categorize_by/split_by_ProcID and "
               "weighted_column_map operations.";
      }

      NUIS_LOGGER_DEBUG("HistFrame", "  -- categorize");
      if (op.category_labels.size()) {
        the_plan.categories_are_labelled = true;
        for (auto const &cat_label : op.category_labels) {
          the_plan.category_cols.push_back(hf.find_column_index(cat_label));
          if (the_plan.category_cols.back() == HistFrame::npos) {
            the_plan.category_cols.back() = hf.add_column(cat_label);
          }
        }
      } else {
        the_plan.categories_are_labelled = false;
      }

      the_plan.categorize_by_colname = op.categorize_by_column;
      the_plan.categorize_by_col =
          eft::require_column_index<int>(ef, the_plan.categorize_by_colname);
      the_plan.fast = false;
    }

    if (op.op == FrameFillOp::kWeightedMap) {

      if (the_plan.weight_map_cols.size()) {
        throw InvalidOperation()
            << "fill passed more than one weighted_column_map operations.";
      }

      if (eft::is_valid_col<EFT>(the_plan.categorize_by_col)) {
        throw InvalidOperation()
            << "fill passed both categorize_by/split_by_ProcID and "
               "weighted_column_map operations.";
      }

      NUIS_LOGGER_DEBUG("HistFrame", "  -- weighted column map");
      for (auto const &weight_col_name : op.weighted_columns) {
        the_plan.weight_map_cols.emplace_back(
            eft::require_column_index<double>(ef, weight_col_name),
            hf.find_column_index(weight_col_name));

        if (the_plan.weight_map_cols.back().second == HistFrame::npos) {
          the_plan.weight_map_cols.back().second =
              hf.add_column(weight_col_name);
        }
      }
      the_plan.fast = false;
    }

    if (op.op == FrameFillOp::kBinIdColumn) {
      if (eft::is_valid_col<EFT>(the_plan.BinId_col) ||
          the_plan.BinIds.size()) {
        throw InvalidOperation()
            << "fill passed more than one prebinned operations.";
      }

      NUIS_LOGGER_DEBUG("HistFrame", "  -- prebinned from column");
      the_plan.BinId_col = eft::require_column_index<int>(ef, op.BinId_column);
    }

    if (op.op == FrameFillOp::kBinIdArray) {

      if (eft::is_valid_col<EFT>(the_plan.BinId_col) ||
          the_plan.BinIds.size()) {
        throw InvalidOperation()
            << "fill passed more than one prebinned operations.";
      }

      NUIS_LOGGER_DEBUG("HistFrame", "  -- prebinned from array");

      int num_rows = eft::num_rows(ef);

      if (num_rows != op.BinIds.size()) {
        throw InvalidOperation()
            << "BinIdArray operation passed with invalid size: "
            << op.BinIds.size()
            << ", it must match "
               "the number of rows in the EventFrame, which has "
            << num_rows << " rows";
      }

      the_plan.BinIds = op.BinIds;
    }
  }

  if (weight_by_CV) {
    the_plan.weight_cols.push_back(
        eft::require_column_index<double>(ef, "weight.cv"));
  }

  for (auto const &proj_col_name : projection_columns) {
    the_plan.proj_cols.push_back(
        eft::require_column_index<double>(ef, proj_col_name));
    nuis_named_log("HistFrame")::log_trace("[fill(EventFrame)]: proj({})",
                                           proj_col_name);
  }

  if (the_plan.default_fill_col == HistFrame::npos) {
    if (!eft::is_valid_col<EFT>(the_plan.categorize_by_col) &&
        !the_plan.weight_map_cols.size()) {
      throw InvalidOperation()
          << "no filled column operations were provided. Expected at least one "
             "of:\n\tfill_column\n\tweighted_column_map\n\tcategorize_"
             "by\n\tsplit_by_ProcID";
    }
  }

  if (the_plan.fast && eft::is_valid_col<EFT>(the_plan.BinId_col) &&
      (the_plan.default_fill_col != HistFrame::npos)) {
    nuis_named_log("HistFrame")::log_trace(
        "[fill(EventFrame)]: Attemping fast_fill");

    if (weight_by_CV && !the_plan.weights.size()) {
      the_plan.BinIds.size() ? fast_fill<true>(hf, ef, the_plan)
                             : fast_fill<false>(hf, ef, the_plan);
      return;
    } else if (weight_by_CV && the_plan.weights.size()) {
      the_plan.BinIds.size() ? fast_fill_weightarray<true>(hf, ef, the_plan)
                             : fast_fill_weightarray<false>(hf, ef, the_plan);
      return;
    } else if (!weight_by_CV && !the_plan.weights.size()) {
      the_plan.BinIds.size() ? fast_fill_noCV<true>(hf, ef, the_plan)
                             : fast_fill_noCV<false>(hf, ef, the_plan);
      return;
    }
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

inline detail::FrameFillOp fill_if_eq(std::string const &cname, int value) {
  detail::FrameFillOp op(detail::FrameFillOp::kConditional);
  op.conditional_column_name = cname;
  op.conditional_value = value;
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

inline detail::FrameFillOp weight_by_array(Eigen::ArrayXdCRef weights) {
  detail::FrameFillOp op(detail::FrameFillOp::kWeightArray);
  op.weights = weights;
  return op;
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
  detail::FrameFillOp op(detail::FrameFillOp::kCategorize);
  op.categorize_by_column = "process.id";
  return op;
}

inline detail::FrameFillOp no_CV_weight() {
  detail::FrameFillOp op(detail::FrameFillOp::kNoCVWeight);
  return op;
}

inline detail::FrameFillOp prebinned(std::string const &cname) {
  detail::FrameFillOp op(detail::FrameFillOp::kBinIdColumn);
  op.BinId_column = cname;
  return op;
}

// use find_bins from Binning/utility.h to calculate binIds
inline detail::FrameFillOp prebinned_array(Eigen::ArrayXiCRef BinIds) {
  detail::FrameFillOp op(detail::FrameFillOp::kBinIdArray);
  op.BinIds = BinIds;
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
