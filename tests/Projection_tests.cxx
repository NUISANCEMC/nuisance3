#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "nuis/histframe/utility.h"

#include "spdlog/spdlog.h"

#include <cassert>
#include <random>

TEST_CASE("2D->1D", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins;

  double xmin = 0;
  double ymin = 0;
  for (size_t yi = 0; yi < 3; ++yi) {
    for (size_t xi = 0; xi < 3; ++xi) {
      in_bins.emplace_back();
      in_bins.back().push_back(nuis::SingleExtent{
          xmin + xi * 1,
          xmin + (xi + 1) * 1,
      });
      in_bins.back().push_back(nuis::SingleExtent{
          ymin + yi * 1,
          ymin + (yi + 1) * 1,
      });
    }
  }

  nuis::HistFrame hf(nuis::Binning::from_extents(in_bins, {"x", "y"}));

  hf.fill({0, 0}, 1);
  hf.fill({1, 0}, 1);
  hf.fill({2, 0}, 1);

  nuis::HistFrame hfp = Project(hf, 0);
  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(0, 0) == 1);
  REQUIRE(hfp.sumweights(1, 0) == 1);
  REQUIRE(hfp.sumweights(2, 0) == 1);
  REQUIRE(hfp.variances(0, 0) == 1);
  REQUIRE(hfp.variances(1, 0) == 1);
  REQUIRE(hfp.variances(2, 0) == 1);

  hf.reset();

  hf.fill({0, 0}, 1);
  hf.fill({1, 0}, 1);
  hf.fill({1, 1}, 1);
  hf.fill({1, 2}, 1);
  hf.fill({2, 0}, 1);

  hfp = Project(hf, 0);
  REQUIRE(hfp.num_fills == 5);
  REQUIRE(hfp.sumweights(0, 0) == 1);
  REQUIRE(hfp.sumweights(1, 0) == 3);
  REQUIRE(hfp.sumweights(2, 0) == 1);
  REQUIRE(hfp.variances(0, 0) == 1);
  REQUIRE(hfp.variances(1, 0) == 3);
  REQUIRE(hfp.variances(2, 0) == 1);

  hfp = Project(hf, 1);
  REQUIRE(hfp.num_fills == 5);
  REQUIRE(hfp.sumweights(0, 0) == 3);
  REQUIRE(hfp.sumweights(1, 0) == 1);
  REQUIRE(hfp.sumweights(2, 0) == 1);
  REQUIRE(hfp.variances(0, 0) == 3);
  REQUIRE(hfp.variances(1, 0) == 1);
  REQUIRE(hfp.variances(2, 0) == 1);

  hf.reset();
}

TEST_CASE("2D swap", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins;

  double xmin = 0;
  double ymin = 0;
  for (size_t yi = 0; yi < 3; ++yi) {
    for (size_t xi = 0; xi < 3; ++xi) {
      in_bins.emplace_back();
      in_bins.back().push_back(nuis::SingleExtent{
          xmin + xi * 1,
          xmin + (xi + 1) * 1,
      });
      in_bins.back().push_back(nuis::SingleExtent{
          ymin + yi * 1,
          ymin + (yi + 1) * 1,
      });
    }
  }

  nuis::HistFrame hf(nuis::Binning::from_extents(in_bins, {"x", "y"}));

  auto bin1 = hf.find_bin({0, 0});
  auto bin2 = hf.find_bin({1, 0});
  auto bin3 = hf.find_bin({2, 0});

  hf.fill_bin(bin1, 1);
  hf.fill_bin(bin2, 1);
  hf.fill_bin(bin3, 1);

  nuis::HistFrame hfp = Project(hf, {1, 0});

  REQUIRE(hfp.binning->axis_labels[0] == "y");
  REQUIRE(hfp.binning->axis_labels[1] == "x");

  auto bin1s = hfp.find_bin({0, 0});
  auto bin2s = hfp.find_bin({0, 1});
  auto bin3s = hfp.find_bin({0, 2});

  REQUIRE(bin1s == bin1);

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1s, 0) == 1);
  REQUIRE(hfp.sumweights(bin2s, 0) == 1);
  REQUIRE(hfp.sumweights(bin3s, 0) == 1);
}

TEST_CASE("3D->2D", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins;

  double xmin = 0;
  double ymin = 0;
  double zmin = 0;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        in_bins.emplace_back();
        in_bins.back().push_back(nuis::SingleExtent{
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        in_bins.back().push_back(nuis::SingleExtent{
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        in_bins.back().push_back(nuis::SingleExtent{
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  nuis::HistFrame hf(nuis::Binning::from_extents(in_bins, {"x", "y", "z"}));

  auto bin1 = hf.find_bin({0, 0, 0});
  auto bin2 = hf.find_bin({1, 1, 0});
  auto bin3 = hf.find_bin({2, 0, 2});

  hf.fill_bin(bin1, 1);
  hf.fill_bin(bin2, 1);
  hf.fill_bin(bin3, 1);

  nuis::HistFrame hfp = Project(hf, {0, 1});

  auto bin1p = hfp.find_bin({0, 0});
  auto bin2p = hfp.find_bin({1, 1});
  auto bin3p = hfp.find_bin({2, 0});

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 1);
  REQUIRE(hfp.sumweights(bin2p, 0) == 1);
  REQUIRE(hfp.sumweights(bin3p, 0) == 1);

  hfp = Project(hf, {0, 2});

  bin1p = hfp.find_bin({0, 0});
  bin2p = hfp.find_bin({1, 0});
  bin3p = hfp.find_bin({2, 2});

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 1);
  REQUIRE(hfp.sumweights(bin2p, 0) == 1);
  REQUIRE(hfp.sumweights(bin3p, 0) == 1);

  hfp = Project(hf, {1, 2});

  bin1p = hfp.find_bin({0, 0});
  bin2p = hfp.find_bin({1, 0});
  bin3p = hfp.find_bin({0, 2});

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 1);
  REQUIRE(hfp.sumweights(bin2p, 0) == 1);
  REQUIRE(hfp.sumweights(bin3p, 0) == 1);
}

TEST_CASE("3D->1D", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins;

  double xmin = 0;
  double ymin = 0;
  double zmin = 0;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        in_bins.emplace_back();
        in_bins.back().push_back(nuis::SingleExtent{
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        in_bins.back().push_back(nuis::SingleExtent{
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        in_bins.back().push_back(nuis::SingleExtent{
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  nuis::HistFrame hf(nuis::Binning::from_extents(in_bins, {"x", "y", "z"}));

  auto bin1 = hf.find_bin({0, 0, 0});
  auto bin2 = hf.find_bin({1, 1, 0});
  auto bin3 = hf.find_bin({2, 0, 2});

  hf.fill_bin(bin1, 1);
  hf.fill_bin(bin2, 1);
  hf.fill_bin(bin3, 1);

  nuis::HistFrame hfp = Project(hf, 0);

  auto bin1p = hfp.find_bin(0);
  auto bin2p = hfp.find_bin(1);
  auto bin3p = hfp.find_bin(2);

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 1);
  REQUIRE(hfp.sumweights(bin2p, 0) == 1);
  REQUIRE(hfp.sumweights(bin3p, 0) == 1);

  hfp = Project(hf, 1);

  bin1p = hfp.find_bin(0);
  bin2p = hfp.find_bin(1);
  bin3p = hfp.find_bin(2);

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 2);
  REQUIRE(hfp.sumweights(bin2p, 0) == 1);
  REQUIRE(hfp.sumweights(bin3p, 0) == 0);

  hfp = Project(hf, 2);

  bin1p = hfp.find_bin(0);
  bin2p = hfp.find_bin(1);
  bin3p = hfp.find_bin(2);

  REQUIRE(hfp.num_fills == 3);
  REQUIRE(hfp.sumweights(bin1p, 0) == 2);
  REQUIRE(hfp.sumweights(bin2p, 0) == 0);
  REQUIRE(hfp.sumweights(bin3p, 0) == 1);
}

TEST_CASE("3D->2D: random", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins3D;
  std::vector<nuis::Binning::BinExtents> in_bins2D;
  std::vector<nuis::Binning::BinExtents> in_bins2Dswap;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        in_bins3D.emplace_back();
        in_bins3D.back().push_back(nuis::SingleExtent{
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        in_bins3D.back().push_back(nuis::SingleExtent{
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        in_bins3D.back().push_back(nuis::SingleExtent{
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });

        if (zi == 0) {
          in_bins2D.emplace_back();
          in_bins2D.back().push_back(nuis::SingleExtent{
              xmin + xi * 1,
              xmin + (xi + 1) * 1,
          });
          in_bins2D.back().push_back(nuis::SingleExtent{
              ymin + yi * 1,
              ymin + (yi + 1) * 1,
          });

          in_bins2Dswap.emplace_back();
          in_bins2Dswap.back().push_back(nuis::SingleExtent{
              ymin + yi * 1,
              ymin + (yi + 1) * 1,
          });
          in_bins2Dswap.back().push_back(nuis::SingleExtent{
              xmin + xi * 1,
              xmin + (xi + 1) * 1,
          });
        }
      }
    }
  }

  nuis::HistFrame hf3D(nuis::Binning::from_extents(in_bins3D, {"x", "y", "z"}));
  nuis::HistFrame hf2D(nuis::Binning::from_extents(in_bins2D, {"x", "y"}));
  nuis::HistFrame hf2Dswap(
      nuis::Binning::from_extents(in_bins2Dswap, {"y", "x"}));

  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_real_distribution<> uni(0, 3);

  for (size_t i = 0; i < 10000; ++i) {
    std::vector<double> rvect = {uni(e1), 3 + uni(e1), 6 + uni(e1)};
    hf3D.fill(rvect, 1);
    hf2D.fill({rvect[0], rvect[1]}, 1);
    hf2Dswap.fill({rvect[1], rvect[0]}, 1);
  }

  nuis::HistFrame hfp = Project(hf3D, {0, 1});
  nuis::HistFrame hfpswap = Project(hf3D, {1, 0});

  REQUIRE(hf3D.num_fills == hf2D.num_fills);
  REQUIRE(hfp.num_fills == hf2D.num_fills);
  REQUIRE(hfpswap.num_fills == hf2Dswap.num_fills);

  for (int i = 0; i < hfp.sumweights.rows(); ++i) {

    if (hfp.sumweights(i, 0) != hf2D.sumweights(i, 0)) {
      spdlog::critical("bin {} didn't match.", i);
      std::stringstream ss;
      ss << "\n-----------\nFilled as 2D:\n"
         << hf2D << "\n-------------\nProjected 3D->2D:\n"
         << hfp << "\n-------------\nFilled as 3D:\n"
         << hf3D;
      spdlog::critical(ss.str());
    }

    REQUIRE(hfp.sumweights(i, 0) == hf2D.sumweights(i, 0));
  }

  for (int i = 0; i < hfpswap.sumweights.rows(); ++i) {

    // do not guarantee bin ordering after projection, so have to search for bin
    // after swap
    auto bin_it =
        std::find(hf2Dswap.binning->bins.begin(), hf2Dswap.binning->bins.end(),
                  hfpswap.binning->bins[i]);
    REQUIRE(bin_it != hf2Dswap.binning->bins.end());
    auto bin_index = std::distance(hf2Dswap.binning->bins.begin(), bin_it);

    if (hfpswap.sumweights(i, 0) != hf2Dswap.sumweights(bin_index, 0)) {
      spdlog::critical("bin {} didn't match.", i);
      std::stringstream ss;
      ss << "\n-----------\nFilled as 2D:\n"
         << hf2Dswap << "\n-------------\nProjected 3D->2D:\n"
         << hfpswap << "\n-------------\nFilled as 3D:\n"
         << hf3D;
      spdlog::critical(ss.str());
    }

    REQUIRE(hfpswap.sumweights(i, 0) == hf2Dswap.sumweights(bin_index, 0));
  }
}

TEST_CASE("3D->1D: random", "[Projection]") {
  std::vector<nuis::Binning::BinExtents> in_bins3D;
  std::vector<nuis::Binning::BinExtents> in_bins1Dx;
  std::vector<nuis::Binning::BinExtents> in_bins1Dy;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        in_bins3D.emplace_back();
        in_bins3D.back().push_back(nuis::SingleExtent{
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        in_bins3D.back().push_back(nuis::SingleExtent{
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        in_bins3D.back().push_back(nuis::SingleExtent{
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });

        if ((zi == 0) && (yi == 0)) {
          in_bins1Dx.emplace_back();
          in_bins1Dx.back().push_back(nuis::SingleExtent{
              xmin + xi * 1,
              xmin + (xi + 1) * 1,
          });
        }
        if ((zi == 0) && (xi == 0)) {
          in_bins1Dy.emplace_back();
          in_bins1Dy.back().push_back(nuis::SingleExtent{
              ymin + yi * 1,
              ymin + (yi + 1) * 1,
          });
        }
      }
    }
  }

  nuis::HistFrame hf3D(nuis::Binning::from_extents(in_bins3D, {"x", "y", "z"}));
  nuis::HistFrame hf1Dx(nuis::Binning::from_extents(in_bins1Dx, {"x"}));
  nuis::HistFrame hf1Dy(nuis::Binning::from_extents(in_bins1Dy, {"y"}));

  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_real_distribution<> uni(0, 3);

  for (size_t i = 0; i < 10000; ++i) {
    std::vector<double> rvect = {uni(e1), 3 + uni(e1), 6 + uni(e1)};
    hf3D.fill(rvect, 1);
    hf1Dx.fill({rvect[0]}, 1);
    hf1Dy.fill({rvect[1]}, 1);
  }

  nuis::HistFrame hfpx = Project(hf3D, 0);
  nuis::HistFrame hfpy = Project(hf3D, 1);

  REQUIRE(hf3D.num_fills == hf1Dx.num_fills);
  REQUIRE(hf3D.num_fills == hf1Dy.num_fills);
  REQUIRE(hfpx.num_fills == hf1Dx.num_fills);
  REQUIRE(hfpy.num_fills == hf1Dy.num_fills);

  for (int i = 0; i < hfpx.sumweights.rows(); ++i) {

    if (hfpx.sumweights(i, 0) != hf1Dx.sumweights(i, 0)) {
      spdlog::critical("bin {} didn't match.", i);
      std::stringstream ss;
      ss << "\n-----------\nFilled as 2D:\n"
         << hf1Dx << "\n-------------\nProjected 3D->1D:\n"
         << hfpx << "\n-------------\nFilled as 3D:\n"
         << hf3D;
      spdlog::critical(ss.str());
    }

    REQUIRE(hfpx.sumweights(i, 0) == hf1Dx.sumweights(i, 0));
  }

  for (int i = 0; i < hfpy.sumweights.rows(); ++i) {

    if (hfpy.sumweights(i, 0) != hf1Dy.sumweights(i, 0)) {
      spdlog::critical("bin {} didn't match.", i);
      std::stringstream ss;
      ss << "\n-----------\nFilled as 2D:\n"
         << hf1Dy << "\n-------------\nProjected 3D->1D:\n"
         << hfpy << "\n-------------\nFilled as 3D:\n"
         << hf3D;
      spdlog::critical(ss.str());
    }

    REQUIRE(hfpy.sumweights(i, 0) == hf1Dy.sumweights(i, 0));
  }
}
