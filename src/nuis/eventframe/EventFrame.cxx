#include "nuis/eventframe/EventFrame.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "nuis/except.h"
#include "nuis/log.txx"

#include <sstream>

namespace nuis {
EventFrame::column_t
EventFrame::find_column_index(std::string const &cn) const {
  auto pos = std::find(column_names.begin(), column_names.end(), cn);
  if (pos == column_names.end()) {
    return EventFrame::npos;
  }
  return pos - column_names.begin();
}

EventFrame::column_t
EventFrame::require_column_index(std::string const &cn) const {
  auto col = find_column_index(cn);
  if (col == EventFrame::npos) {
    throw InvalidFrameColumnName()
        << fmt::format("require_column_index(column=\"{}\"), but no such "
                       "column exists. Valid columns: {}",
                       cn, column_names);
  }
  return col;
}

Eigen::ArrayXdRef EventFrame::col(std::string const &cn) {
  auto cid = find_column_index(cn);
  if (cid == EventFrame::npos) {
    nuis_named_log("EventFrame")::log_critical(
        "Tried to get column, named {} from Eventframe. But no such column "
        "exists.",
        cn);
    throw InvalidFrameColumnName();
  }
  return table.col(cid);
}

Eigen::ArrayXdCRef EventFrame::col(std::string const &cn) const {
  auto cid = find_column_index(cn);
  if (cid == EventFrame::npos) {
    nuis_named_log("EventFrame")::log_critical(
        "Tried to get column, named {} from Eventframe. But no such column "
        "exists.",
        cn);
    throw InvalidFrameColumnName();
  }
  return table.col(cid);
}

std::vector<Eigen::ArrayXdRef>
EventFrame::cols(std::vector<std::string> const &cns) {
  std::vector<Eigen::ArrayXdRef> rtn;
  for (size_t i = 0; i < cns.size(); ++i) {
    auto cid = find_column_index(cns[i]);
    if (cid == EventFrame::npos) {
      nuis_named_log("EventFrame")::log_critical(
          "Tried to get column, named {} from Eventframe. But no such "
          "column exists.",
          cns[i]);
      throw InvalidFrameColumnName();
    }
    rtn.push_back(table.col(cid));
  }
  return rtn;
}

std::ostream &operator<<(std::ostream &os, nuis::EventFramePrinter fp) {

  size_t abs_max_width = fp.max_col_width;

  auto const &f = fp.fr.get();

  if (!fp.prettyprint) {
    return os << f.table.topRows(fp.max_rows);
  }

  std::vector<size_t> col_widths(f.table.cols(), 0);

  // check up to the first 20 rows to guess how wide we need each column
  for (int ri = 0; ri < f.table.rows(); ++ri) {
    for (int ci = 0; ci < f.table.cols(); ++ci) {
      std::string test = fmt::format("{:>.4}", f.table(ri, ci));
      size_t len = test.size() - test.find_first_not_of(" ");
      col_widths[ci] = std::min(std::max(col_widths[ci], len), abs_max_width);
    }
    if (ri >= 20) {
      break;
    }
  }

  std::stringstream hdr;
  std::vector<std::string> fmtstrs;
  hdr << " |";

  for (size_t ci = 0; ci < f.column_names.size(); ++ci) {
    std::string cn = (f.column_names[ci].size() > abs_max_width)
                         ? f.column_names[ci].substr(0, abs_max_width - 1) + "$"
                         : f.column_names[ci];

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

  for (int ri = 0; ri < f.table.rows(); ++ri) {
    os << " |";
    for (int ci = 0; ci < f.table.cols(); ++ci) {
      os << fmt::format(fmtstrs[ci], f.table(ri, ci));
    }
    os << std::endl;
    if (ri >= fp.max_rows) {
      os << " |";
      for (int ci = 0; ci < f.table.cols(); ++ci) {
        os << fmt::format(fmtstrs[ci], "...");
      }
      os << std::endl;
      break;
    }
  }

  return os << " " << line.data();
}

std::ostream &operator<<(std::ostream &os, nuis::EventFrame const &f) {
  return os << nuis::EventFramePrinter(f);
}

} // namespace nuis
