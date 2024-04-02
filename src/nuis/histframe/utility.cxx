#include "nuis/histframe/utility.h"
#include "nuis/histframe/exceptions.h"

#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"

#include "fmt/ranges.h"

#include "nuis/log.txx"

namespace nuis {

struct ProjectionMap {
  std::vector<size_t> project_to_axes;
  std::vector<Binning::BinExtents> projected_extents;
  std::vector<std::vector<Binning::index_t>> bin_columns;
};

ProjectionMap BuildProjectionMap(BinningPtr const bin_info,
                                 std::vector<size_t> proj_to_axes) {
  ProjectionMap pm;
  pm.project_to_axes = proj_to_axes;
  pm.projected_extents = project_to_unique_bins(bin_info->bins, proj_to_axes);

  for (size_t i = 0; i < pm.projected_extents.size(); ++i) {
    pm.bin_columns.emplace_back(std::vector<Binning::index_t>{});
  }

  for (Binning::index_t bi_it = 0;
       bi_it < Binning::index_t(bin_info->bins.size()); ++bi_it) {

    auto const &bin = bin_info->bins[bi_it];

    Binning::BinExtents proj_bin;
    for (auto proj_to_axis : proj_to_axes) {
      proj_bin.push_back(bin[proj_to_axis]);
    }

    auto bin_it = std::find(pm.projected_extents.begin(),
                            pm.projected_extents.end(), proj_bin);

    if (bin_it == pm.projected_extents.end()) {
      log_critical(
          "[BuildProjectionMap]: When scanning bins, built projected bin "
          "extent that project_to_unique_bins did not find, this is a bug in "
          "NUISANCE, please report it to the authors.");
      std::stringstream ss;
      ss << "REPORT INFO:\n>>>----------------------------\ninput bin_info:\n"
         << bin_info << "\n";
      log_critical(ss.str());
      ss.str("");
      ss << "projected extents: " << pm.projected_extents << "\n";
      log_critical(ss.str());
      ss.str("");
      ss << "missed bin: " << proj_bin << "\n----------------------------<<<\n";
      throw CatastrophicBinningFailure();
    }
    pm.bin_columns[std::distance(pm.projected_extents.begin(), bin_it)]
        .push_back(bi_it);
  }

  return pm;
}

} // namespace nuis

std::ostream &operator<<(std::ostream &os, nuis::ProjectionMap const &pm) {
  os << "{ Project onto axis: " << pm.project_to_axes.front() << "\n";
  for (size_t i = 0; i < pm.projected_extents.size(); ++i) {
    os << "  { projected bin: " << (i)
       << ", extent: " << pm.projected_extents[i]
       << fmt::format(", original_bins: {} }}\n", pm.bin_columns[i]);
  }
  return os << "}" << std::endl;
}

namespace nuis {

template <typename T>
T Project_impl(T const &histlike, std::vector<size_t> const &proj_to_axes) {
  auto const &pm = BuildProjectionMap(histlike.binning, proj_to_axes);

  std::vector<std::string> labels;
  for (auto proj_to_axis : proj_to_axes) {
    labels.push_back(histlike.binning->axis_labels[proj_to_axis]);
  }

  T projhl(Binning::from_extents(pm.projected_extents, labels));
  projhl.column_info = histlike.column_info;
  projhl.resize();

  for (size_t row_i = 0; row_i < pm.projected_extents.size(); ++row_i) {
    for (Binning::index_t bi_it : pm.bin_columns[row_i]) {
      if constexpr (std::is_same_v<T, HistFrame>) {
        projhl.sumweights.row(row_i) += histlike.sumweights.row(bi_it);
        projhl.variances.row(row_i) += histlike.variances.row(bi_it);
      } else if (std::is_same_v<T, BinnedValues>) {
        projhl.values.row(row_i) += histlike.values.row(bi_it);
        projhl.errors.row(row_i) += histlike.errors.row(bi_it).square();
      }
    }
  }
  if constexpr (std::is_same_v<T, BinnedValues>) {
    projhl.errors.sqrt();
  }

  if constexpr (std::is_same_v<T, HistFrame>) {
    projhl.num_fills = histlike.num_fills;
  }

  return projhl;
}

HistFrame Project(HistFrame const &hf,
                  std::vector<size_t> const &proj_to_axes) {
  return Project_impl<HistFrame>(hf, proj_to_axes);
}
HistFrame Project(HistFrame const &hf, size_t proj_to_axis) {
  return Project(hf, std::vector<size_t>{proj_to_axis});
}

BinnedValues Project(BinnedValues const &hf,
                     std::vector<size_t> const &proj_to_axes) {
  return Project_impl<BinnedValues>(hf, proj_to_axes);
}
BinnedValues Project(BinnedValues const &hf, size_t proj_to_axis) {
  return Project(hf, std::vector<size_t>{proj_to_axis});
}

std::ostream &operator<<(std::ostream &os, nuis::BinnedValuesBase const &bvb) {

  size_t abs_max_width = 12;

  auto contents = bvb.get_bin_contents();
  auto errors = bvb.get_bin_uncertainty();

  std::vector<size_t> col_widths(contents.cols() * 2, 0);

  // check up to the first 20 rows to guess how wide we need each column
  for (int ri = 0; ri < contents.rows(); ++ri) {
    for (int ci = 0; ci < (contents.cols() * 2); ++ci) {

      double v = (ci & 1) ? errors(ri, ci / 2) : (contents(ri, ci / 2));
      std::string test = fmt::format("{:>.4}", v);

      size_t len = test.size() - test.find_first_not_of(" ");
      col_widths[ci] = std::min(std::max(col_widths[ci], len), abs_max_width);
    }
    if (ri >= 20) {
      break;
    }
  }

  std::stringstream hdr;
  std::vector<std::string> fmtstrs;
  hdr << " | bin |";

  for (size_t ci = 0; ci < (bvb.column_info.size() * 2); ++ci) {
    std::string cfull =
        (ci & 1) ? std::string("err") : bvb.column_info[ci / 2].name;
    std::string cn = (cfull.size() > abs_max_width)
                         ? cfull.substr(0, abs_max_width - 1) + "$"
                         : cfull;

    col_widths[ci] = std::max(col_widths[ci], cn.size());

    hdr << fmt::format(" {:>" + std::to_string(col_widths[ci]) + "} |", cn);
    fmtstrs.push_back(" {:>" + std::to_string(col_widths[ci]) + ".4} |");
  }

  std::string hdrs = hdr.str();

  std::vector<char> line(hdrs.size() + 1, '-');
  line[hdrs.size() - 1] = '\0';
  os << " " << line.data() << std::endl;
  os << hdrs << std::endl;
  os << " " << line.data() << std::endl;

  for (int ri = 0; ri < contents.rows(); ++ri) {
    os << fmt::format(" | {:>3} |", ri);
    for (int ci = 0; ci < (contents.cols() * 2); ++ci) {
      double v = (ci & 1) ? errors(ri, ci / 2) : contents(ri, ci / 2);
      os << fmt::format(fmtstrs[ci], v);
    }
    os << std::endl;
    if (ri >= 20) {
      os << " |";
      for (int ci = 0; ci < (contents.cols() * 2); ++ci) {
        os << fmt::format(fmtstrs[ci], "...");
      }
      os << std::endl;
      break;
    }
  }

  return os << " " << line.data();
}

template <bool fill_columns, bool fill_if, bool autoprocidcolumns,
          bool autoweightcolumns>
void fill_procid_columns_from_EventFrame_if_impl(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {

  static_assert(!(fill_columns && autoprocidcolumns),
                "Cannot use EventFrame-filling utility function with both "
                "column filler and automatic procid columns at the same time.");

  static_assert(!(fill_columns && autoweightcolumns),
                "Cannot use EventFrame-filling utility function with both "
                "column filler and column weighter at the same time.");

  static_assert(
      !(autoprocidcolumns && autoweightcolumns),
      "Cannot use EventFrame-filling utility function with both "
      "automatic procid columns and column weighter at the same time.");

  EventFrame::column_t cond_col;
  if constexpr (fill_if) {
    cond_col = ef.find_column_index(conditional_column_name);
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_EventFrame_if]: cond({}) -> {}",
        conditional_column_name, cond_col);

    if (cond_col == EventFrame::npos) {
      throw InvalidColumnAccess() << conditional_column_name
                                  << " column does not exist in EventFrame.\n"
                                  << ef;
    }
  }

  EventFrame::column_t colsel_col;
  if constexpr (fill_columns) {
    colsel_col = ef.find_column_index(column_selector_column_name);
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_EventFrame_if]: col({}) -> {}",
        column_selector_column_name, colsel_col);

    if (colsel_col == EventFrame::npos) {
      throw InvalidColumnAccess() << column_selector_column_name
                                  << " column does not exist in EventFrame.\n"
                                  << ef;
    }
  }

  std::vector<double> projs(projection_column_names.size(), 0);
  double weight = 1;

  std::vector<EventFrame::column_t> proj_colids;
  for (auto const &proj_col_name : projection_column_names) {
    proj_colids.push_back(ef.find_column_index(proj_col_name));
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_EventFrame_if]: proj({}) -> {}", proj_col_name,
        proj_colids.back());
  }

  std::vector<EventFrame::column_t> weight_colids;
  for (auto const &weight_col_name : weight_column_names) {
    weight_colids.push_back(ef.find_column_index(weight_col_name));
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_EventFrame_if]: wght({}) -> {}", weight_col_name,
        weight_colids.back());
  }

  std::vector<EventFrame::column_t> auto_weight_ecolids;
  std::vector<HistFrame::column_t> auto_weight_hcolids;
  if constexpr (autoweightcolumns) {
    for (auto const &weight_col_name : column_weighter_names) {
      auto_weight_ecolids.push_back(ef.find_column_index(weight_col_name));

      auto hf_colid = hf.find_column_index(weight_col_name);
      if (hf_colid == HistFrame::npos) {
        auto_weight_hcolids.push_back(hf.add_column(weight_col_name));
      } else {
        auto_weight_hcolids.push_back(hf_colid);
      }
    }
  }

  EventFrame::column_t procid_col;

  std::vector<int> proc_id_dictionary;
  if constexpr (autoprocidcolumns) {

    procid_col = ef.find_column_index("process.id");
    nuis_named_log("HistFrame")::log_trace(
        "[fill_procid_columns_from_EventFrame_if]: col({}) -> {}", "process.id",
        procid_col);

    if (procid_col == EventFrame::npos) {
      throw InvalidColumnAccess()
          << "process.id" << " column does not exist in EventFrame.\n"
          << ef;
    }

    // fill the dictionary with existing column names so that repeated calls
    // with EventFrame batches don't result in repeated columns
    for (HistFrame::column_t col_it = 1; col_it < hf.column_info.size();
         ++col_it) {
      try {
        proc_id_dictionary.push_back(std::stoi(hf.column_info[col_it].name));
      } catch (std::invalid_argument const &ia) {
        throw InvalidColumnName()
            << "[fill_procid_columns_from_EventFrame_if]: Encountered "
               "HistFrame column named "
            << hf.column_info[col_it].name
            << " which is not castable to an integer. "
               "fill_procid_columns_from_EventFrame_if should only be used "
               "with HistFrames with no manually added columns, or ones that "
               "have already been filled with "
               "fill_procid_columns_from_EventFrame_if.\n"
            << ia.what();
      }
    }
  }

  for (auto row : ef.table.rowwise()) {

    if constexpr (fill_if) {
      if (row(cond_col) == 0) {
        continue;
      }
    }

    weight = 1;
    for (auto wid : weight_colids) {
      weight *= row(wid);
    }

    for (size_t pi = 0; pi < proj_colids.size(); ++pi) {
      projs[pi] = row(proj_colids[pi]);
    }

    auto bin = hf.find_bin(projs);
    hf.fill_bin(bin, weight, 0);

    if constexpr (fill_columns) {
      HistFrame::column_t col = row(colsel_col);
      if (col > 0) {
        hf.fill_bin(bin, weight, col);
      }
    }

    if constexpr (autoprocidcolumns) {
      int colv = row(procid_col);
      auto it =
          std::find(proc_id_dictionary.begin(), proc_id_dictionary.end(), colv);

      if (it == proc_id_dictionary.end()) {
        proc_id_dictionary.push_back(colv);
        hf.add_column(fmt::format("{}", colv));
      }

      HistFrame::column_t col = 1 + (it - proc_id_dictionary.begin());

      hf.fill_bin(bin, weight, col);
    }

    if constexpr (autoweightcolumns) {
      for (size_t col_it = 0; col_it < auto_weight_ecolids.size(); ++col_it) {
        hf.fill_bin(bin, weight * row(auto_weight_ecolids[col_it]),
                    auto_weight_hcolids[col_it]);
      }
    }
  }
}

void fill_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, false, false, false>(
      hf, ef, "", projection_column_names, "", {}, weight_column_names);
}

void fill_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, true, false, false>(
      hf, ef, conditional_column_name, projection_column_names, "", {},
      weight_column_names);
}

void fill_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<true, false, false, false>(
      hf, ef, "", projection_column_names, column_selector_column_name, {},
      weight_column_names);
}

void fill_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<true, true, false, false>(
      hf, ef, conditional_column_name, projection_column_names,
      column_selector_column_name, {}, weight_column_names);
}

void fill_weighted_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, false, false, true>(
      hf, ef, "", projection_column_names, "", column_weighter_names,
      weight_column_names);
}

void fill_weighted_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, true, false, true>(
      hf, ef, conditional_column_name, projection_column_names, "",
      column_weighter_names, weight_column_names);
}

void fill_procid_columns_from_EventFrame(
    HistFrame &hf, EventFrame const &ef,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, false, true, false>(
      hf, ef, "", projection_column_names, "", {}, weight_column_names);
}

void fill_procid_columns_from_EventFrame_if(
    HistFrame &hf, EventFrame const &ef,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_EventFrame_if_impl<false, true, true, false>(
      hf, ef, conditional_column_name, projection_column_names, "", {},
      weight_column_names);
}

void fill_from_EventFrameGen(
    HistFrame &hf, EventFrameGen &efg,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  auto fr = efg.first();
  while (fr.table.rows()) {
    fill_from_EventFrame(hf, fr, projection_column_names, weight_column_names);
    fr = efg.next();
  }
}

#ifdef NUIS_ARROW_ENABLED

template <bool fill_columns, bool fill_if, bool autoprocidcolumns,
          bool autoweightcolumns>
void fill_procid_columns_from_RecordBatch_if_impl(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {

  static_assert(!(fill_columns && autoprocidcolumns),
                "Cannot use RecordBatch-filling utility function with both "
                "column filler and automatic procid columns at the same time.");

  static_assert(!(fill_columns && autoweightcolumns),
                "Cannot use RecordBatch-filling utility function with both "
                "column filler and column weighter at the same time.");

  static_assert(
      !(autoprocidcolumns && autoweightcolumns),
      "Cannot use RecordBatch-filling utility function with both "
      "automatic procid columns and column weighter at the same time.");

  std::function<bool(int)> cond_col;
  if constexpr (fill_if) {
    cond_col = get_col_cast_to<bool>(rb, conditional_column_name);
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_RecordBatch_if]: cond({})",
        conditional_column_name);
  }

  std::function<int(int)> colsel_col;
  if constexpr (fill_columns) {
    colsel_col = get_col_cast_to<int>(rb, column_selector_column_name);
    nuis_named_log("HistFrame")::log_trace(
        "[fill_columns_from_RecordBatch_if]: col({})",
        column_selector_column_name);
  }

  std::vector<double> projs(projection_column_names.size(), 0);
  double weight = 1;

  std::vector<std::function<double(int)>> proj_columns;
  for (auto const &proj_col_name : projection_column_names) {
    proj_columns.push_back(get_col_cast_to<double>(rb, proj_col_name));
  }

  std::vector<std::function<double(int)>> weight_columns;
  for (auto const &weight_col_name : weight_column_names) {
    weight_columns.push_back(get_col_cast_to<double>(rb, weight_col_name));
  }

  std::vector<std::function<double(int)>> auto_weight_columns;
  std::vector<HistFrame::column_t> auto_weight_hcolids;
  if constexpr (autoweightcolumns) {
    for (auto const &weight_col_name : column_weighter_names) {
      auto_weight_columns.push_back(
          get_col_cast_to<double>(rb, weight_col_name));

      auto hf_colid = hf.find_column_index(weight_col_name);
      if (hf_colid == HistFrame::npos) {
        auto_weight_hcolids.push_back(hf.add_column(weight_col_name));
      } else {
        auto_weight_hcolids.push_back(hf_colid);
      }
    }
  }

  std::function<int(int)> procid_col;

  std::vector<int> proc_id_dictionary;
  if constexpr (autoprocidcolumns) {
    procid_col = get_col_cast_to<int>(rb, "process.id");

    // fill the dictionary with existing column names so that repeated calls
    // with RecordBatch batches don't result in repeated columns
    for (HistFrame::column_t col_it = 1; col_it < hf.column_info.size();
         ++col_it) {
      try {
        proc_id_dictionary.push_back(std::stoi(hf.column_info[col_it].name));
      } catch (std::invalid_argument const &ia) {
        throw InvalidColumnName()
            << "[fill_procid_columns_from_RecordBatch_if]: Encountered "
               "HistFrame column named "
            << hf.column_info[col_it].name
            << " which is not castable to an integer. "
               "fill_procid_columns_from_RecordBatch_if should only be used "
               "with HistFrames with no manually added columns, or ones that "
               "have already been filled with "
               "fill_procid_columns_from_RecordBatch_if.\n"
            << ia.what();
      }
    }
  }

  for (int row_it = 0; row_it < rb->num_rows(); ++row_it) {

    if constexpr (fill_if) {
      if (cond_col(row_it) == 0) {
        continue;
      }
    }

    weight = 1;
    for (auto &wcol : weight_columns) {
      weight *= wcol(row_it);
    }

    for (size_t pi = 0; pi < proj_columns.size(); ++pi) {
      projs[pi] = proj_columns[pi](row_it);
    }

    auto bin = hf.find_bin(projs);
    hf.fill_bin(bin, weight, 0);

    if constexpr (fill_columns) {
      HistFrame::column_t col = colsel_col(row_it);
      if (col > 0) {
        hf.fill_bin(bin, weight, col);
      }
    }

    if constexpr (autoprocidcolumns) {
      int colv = procid_col(row_it);
      auto it =
          std::find(proc_id_dictionary.begin(), proc_id_dictionary.end(), colv);

      if (it == proc_id_dictionary.end()) {
        proc_id_dictionary.push_back(colv);
        hf.add_column(fmt::format("{}", colv));
      }

      HistFrame::column_t col = 1 + (it - proc_id_dictionary.begin());

      hf.fill_bin(bin, weight, col);
    }

    if constexpr (autoweightcolumns) {
      for (size_t col_it = 0; col_it < auto_weight_hcolids.size(); ++col_it) {
        hf.fill_bin(bin, weight * auto_weight_columns[col_it](row_it),
                    auto_weight_hcolids[col_it]);
      }
    }
  }
}

void fill_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, false, false, false>(
      hf, rb, "", projection_column_names, "", {}, weight_column_names);
}

void fill_from_RecordBatch_if(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, true, false, false>(
      hf, rb, conditional_column_name, projection_column_names, "", {},
      weight_column_names);
}

void fill_columns_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<true, false, false, false>(
      hf, rb, "", projection_column_names, column_selector_column_name, {},
      weight_column_names);
}

void fill_columns_from_RecordBatch_if(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::string const &column_selector_column_name,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<true, true, false, false>(
      hf, rb, conditional_column_name, projection_column_names,
      column_selector_column_name, {}, weight_column_names);
}

void fill_weighted_columns_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, false, false, true>(
      hf, rb, "", projection_column_names, "", column_weighter_names,
      weight_column_names);
}

void fill_weighted_columns_from_RecordBatch_if(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &column_weighter_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, true, false, true>(
      hf, rb, conditional_column_name, projection_column_names, "",
      column_weighter_names, weight_column_names);
}

void fill_procid_columns_from_RecordBatch(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, false, true, false>(
      hf, rb, "", projection_column_names, "", {}, weight_column_names);
}

void fill_procid_columns_from_RecordBatch_if(
    HistFrame &hf, std::shared_ptr<arrow::RecordBatch> const &rb,
    std::string const &conditional_column_name,
    std::vector<std::string> const &projection_column_names,
    std::vector<std::string> const &weight_column_names) {
  fill_procid_columns_from_RecordBatch_if_impl<false, true, true, false>(
      hf, rb, conditional_column_name, projection_column_names, "", {},
      weight_column_names);
}
#endif

} // namespace nuis
