#include "nuis/frame/Frame.h"

#include "fmt/core.h"

#include <sstream>

std::ostream &operator<<(std::ostream &os, nuis::Frame const &f) {

  size_t abs_max_width = 12;
  std::vector<size_t> col_widths(f.Table.cols(), 0);

  // check up to the first 20 rows to guess how wide we need each column
  for (int ri = 0; ri < f.Table.rows(); ++ri) {
    for (int ci = 0; ci < f.Table.cols(); ++ci) {
      std::string test = fmt::format("{:>.4}", f.Table(ri, ci));
      size_t len = test.size() - test.find_first_not_of(" ");
      col_widths[ci] = std::min(std::max(col_widths[ci], len), abs_max_width);
    }
    if (ri > 20) {
      break;
    }
  }

  std::stringstream hdr;
  std::vector<std::string> fmtstrs;
  hdr << " |";

  for (size_t ci = 0; ci < f.ColumnNames.size(); ++ci) {
    std::string cn = (f.ColumnNames[ci].size() > abs_max_width)
                         ? f.ColumnNames[ci].substr(0, abs_max_width - 1) + "$"
                         : f.ColumnNames[ci];

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

  for (int ri = 0; ri < f.Table.rows(); ++ri) {
    os << " |";
    for (int ci = 0; ci < f.Table.cols(); ++ci) {
      os << fmt::format(fmtstrs[ci], f.Table(ri, ci));
    }
    os << std::endl;
  }

  return os << " " << line.data();
}