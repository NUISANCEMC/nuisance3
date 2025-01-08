#include "nuis/convert/misc.h"

#include "nuis/binning/utility.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

namespace nuis {

std::tuple<Eigen::ArrayXXd, Eigen::ArrayXXd, Eigen::ArrayXXd>
to_mpl_pcolormesh(BinnedValuesBase const &bvb, BinnedValuesBase::column_t col) {

  auto const &[x_edges, y_edges] = get_rectilinear_grid(bvb.binning->bins);

  Eigen::ArrayXXd X, Y, C;
  X = Eigen::ArrayXXd::Zero(x_edges.size(), y_edges.size());
  Y = Eigen::ArrayXXd::Zero(x_edges.size(), y_edges.size());
  C = Eigen::ArrayXXd::Zero(x_edges.size() - 1, y_edges.size() - 1);

  auto bin_contents = bvb.get_bin_contents();

  for (size_t x = 0; x < x_edges.size(); ++x) {
    for (size_t y = 0; y < y_edges.size(); ++y) {
      X(x, y) = x_edges[x];
      Y(x, y) = y_edges[y];
      if (((x + 1) < x_edges.size()) && ((y + 1) < y_edges.size())) {

        auto bi_it =
            bvb.binning->find_bin({(x_edges[x] + x_edges[x + 1]) / 2.0,
                                   (y_edges[y] + y_edges[y + 1]) / 2.0});

        if (bi_it == Binning::npos) {
          C(x, y) = std::numeric_limits<double>::quiet_NaN();

        } else {
          C(x, y) = bin_contents(bi_it, col);
        }
      }
    }
  }

  return {X, Y, C};
}

} // namespace nuis
