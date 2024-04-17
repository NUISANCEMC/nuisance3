#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "nuis/binning/Binning.h"
#include "nuis/binning/exceptions.h"
#include "nuis/log.txx"

#include <cassert>

TEST_CASE("SingleExtent::width", "[Binning]") {
  nuis::SingleExtent se{1, 2};

  REQUIRE(se.width() == 1);
}

TEST_CASE("SingleExtent::operator==", "[Binning]") {
  nuis::SingleExtent se1{1, 2};
  nuis::SingleExtent se2{1, 2};
  nuis::SingleExtent se3{0.9, 2};

  REQUIRE(se1 == se1);
  REQUIRE(se1 == se2);
  REQUIRE_FALSE(se1 == se3);
}

TEST_CASE("SingleExtent::operator<", "[Binning]") {
  nuis::SingleExtent se1{1, 2};
  nuis::SingleExtent se2{1, 2};
  nuis::SingleExtent se3{0.9, 2};
  nuis::SingleExtent se4{0.9, 1.8};
  nuis::SingleExtent se5{1, 2.2};
  nuis::SingleExtent se6{1.1, 2.2};

  REQUIRE_FALSE(se1 < se1);
  REQUIRE_FALSE(se1 < se2);
  REQUIRE_FALSE(se2 < se1);
  REQUIRE(se3 < se1);
  REQUIRE(se4 < se1);
  REQUIRE(se1 < se5);
  REQUIRE_FALSE(se5 < se1);
  REQUIRE(se1 < se6);
  REQUIRE_FALSE(se6 < se1);
}

TEST_CASE("SingleExtent::overlaps", "[Binning]") {
  nuis::SingleExtent se1{1, 2};
  nuis::SingleExtent se2{1, 2};
  nuis::SingleExtent se3{0.9, 2};
  nuis::SingleExtent se4{0.9, 1.8};
  nuis::SingleExtent se5{1, 2.2};
  nuis::SingleExtent se6{1.1, 2.2};
  nuis::SingleExtent se7{0, 1};
  nuis::SingleExtent se8{2, 3};

  REQUIRE(se1.overlaps(se1));
  REQUIRE(se1.overlaps(se2));
  REQUIRE(se2.overlaps(se1));
  REQUIRE(se3.overlaps(se1));
  REQUIRE(se4.overlaps(se1));
  REQUIRE(se1.overlaps(se5));
  REQUIRE(se5.overlaps(se1));
  REQUIRE(se1.overlaps(se6));
  REQUIRE(se6.overlaps(se1));

  REQUIRE_FALSE(se1.overlaps(se7));
  REQUIRE_FALSE(se1.overlaps(se8));
  REQUIRE_FALSE(se7.overlaps(se1));
  REQUIRE_FALSE(se8.overlaps(se1));
}

TEST_CASE("SingleExtent::contains", "[Binning]") {
  nuis::SingleExtent se{1, 2};

  REQUIRE(se.contains(1));
  REQUIRE(se.contains(1.5));
  REQUIRE_FALSE(se.contains(0));
  REQUIRE_FALSE(se.contains(2));
  REQUIRE_FALSE(se.contains(3));
}

TEST_CASE("BinExtents::operator<", "[Binning]") {
  nuis::Binning::BinExtents ex_0_0{{0, 1}, {0, 1}};
  nuis::Binning::BinExtents ex_1_0{{1, 2}, {0, 1}};
  nuis::Binning::BinExtents ex_0_1{{0, 1}, {1, 2}};
  nuis::Binning::BinExtents ex_1_1{{1, 2}, {1, 2}};

  REQUIRE_FALSE(ex_0_0 < ex_0_0);
  REQUIRE(ex_0_0 < ex_1_0);
  REQUIRE(ex_0_0 < ex_0_1);
  REQUIRE(ex_0_0 < ex_1_1);

  REQUIRE(ex_1_0 < ex_0_1);
  REQUIRE(ex_1_0 < ex_1_1);

  REQUIRE(ex_0_1 < ex_1_1);
}

TEST_CASE("BinExtents::operator< mismatched axis count", "[Binning]") {
  nuis::Binning::BinExtents a{{0, 1}, {0, 1}, {0, 1}};
  nuis::Binning::BinExtents b{{1, 2}, {0, 1}};

  REQUIRE_THROWS_AS(a < b, nuis::MismatchedAxisCount);
}

TEST_CASE("lin_space::axis_labels", "[Binning]") {
  auto ls = nuis::Binning::lin_space(0, 5, 5, "x");
  REQUIRE(ls->axis_labels.size() == 1);
  REQUIRE(ls->axis_labels[0] == "x");
}

TEST_CASE("lin_space::bins", "[Binning]") {
  auto ls = nuis::Binning::lin_space(0, 5, 5);
  auto bins = ls->bins;

  REQUIRE(bins.size() == 5);
  REQUIRE(bins.front().size() == 1);
  REQUIRE(bins.front().front().low == 0);
  REQUIRE(bins.front().front().high == 1);
  REQUIRE(bins.back().size() == 1);
  REQUIRE(bins.back().front().low == 4);
  REQUIRE(bins.back().front().high == 5);
}

TEST_CASE("lin_space::func", "[Binning]") {
  auto ls = nuis::Binning::lin_space(0, 5, 5);

  REQUIRE(ls->find_bin(-1) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(-0.000001) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(0) == 0);
  REQUIRE(ls->find_bin(1) == 1);
  REQUIRE(ls->find_bin(5) == nuis::Binning::npos);
}

TEST_CASE("lin_spaceND::axis_labels", "[Binning]") {
  auto ls = nuis::Binning::lin_spaceND({{0, 3, 3}, {3, 6, 3}, {6, 9, 3}},
                                       {"x", "y", "z"});
  REQUIRE(ls->axis_labels.size() == 3);
  REQUIRE(ls->axis_labels[0] == "x");
  REQUIRE(ls->axis_labels[2] == "z");
}

TEST_CASE("lin_spaceND::bins", "[Binning]") {
  auto ls = nuis::Binning::lin_spaceND({{0, 3, 3}, {3, 6, 3}, {6, 9, 3}},
                                       {"x", "y", "z"});
  auto bins = ls->bins;

  REQUIRE(bins.size() == 27);
  REQUIRE(bins.front().size() == 3);
  REQUIRE(bins.front().front().low == 0);
  REQUIRE(bins.front().front().high == 1);
  REQUIRE(bins.front().back().low == 6);
  REQUIRE(bins.front().back().high == 7);

  REQUIRE(bins.back().size() == 3);
  REQUIRE(bins.back().front().low == 2);
  REQUIRE(bins.back().front().high == 3);
  REQUIRE(bins.back().back().low == 8);
  REQUIRE(bins.back().back().high == 9);
}

TEST_CASE("lin_spaceND::func", "[Binning]") {
  auto ls = nuis::Binning::lin_spaceND({{0, 3, 3}, {3, 6, 3}, {6, 9, 3}},
                                       {"x", "y", "z"});

  REQUIRE(ls->find_bin({0, 3, 6}) == 0);
  REQUIRE(ls->find_bin({1, 3, 6}) == 1);
  REQUIRE(ls->find_bin({1, 3, 7}) == 10);
  REQUIRE(ls->find_bin({1, 4, 7}) == 13);
  REQUIRE(ls->find_bin({2, 5, 8}) == 26);

  // OOR above
  REQUIRE(ls->find_bin({3, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 6, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 9}) == nuis::Binning::npos);

  // OOR below
  REQUIRE(ls->find_bin({-0.1, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 2.9, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 5.9}) == nuis::Binning::npos);
}

TEST_CASE("log10_space::axis_labels", "[Binning]") {
  auto ls = nuis::Binning::log10_space(1, 1E3, 3, "lx");
  REQUIRE(ls->axis_labels.size() == 1);
  REQUIRE(ls->axis_labels[0] == "lx");
}

TEST_CASE("log10_space::bins", "[Binning]") {
  auto ls = nuis::Binning::log10_space(1, 1E3, 3, "lx");
  auto bins = ls->bins;

  REQUIRE(bins.size() == 3);
  REQUIRE(bins.front().size() == 1);
  REQUIRE_THAT(bins.front().front().low, Catch::Matchers::WithinRel(1, 1E-8));
  REQUIRE_THAT(bins.front().front().high, Catch::Matchers::WithinRel(10, 1E-8));
  REQUIRE_THAT(bins.back().front().low, Catch::Matchers::WithinRel(100, 1E-8));
  REQUIRE_THAT(bins.back().front().high,
               Catch::Matchers::WithinRel(1000, 1E-8));
}

TEST_CASE("log10_space::func", "[Binning]") {
  auto ls = nuis::Binning::log10_space(1, 1E3, 3, "lx");

#ifndef NUIS_NDEBUG
  REQUIRE_THROWS_AS(ls->find_bin(0), nuis::UnbinnableNumber);
#else
  REQUIRE(ls->find_bin(0) == nuis::Binning::npos);
#endif

  REQUIRE(ls->find_bin(0.1) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(1) == 0);
  REQUIRE(ls->find_bin(10) == 1);
  REQUIRE(ls->find_bin(10) == 1);
  REQUIRE(ls->find_bin(100) == 2);
  REQUIRE(ls->find_bin(1000) == nuis::Binning::npos);
}

TEST_CASE("contiguous::axis_labels", "[Binning]") {
  auto ls = nuis::Binning::contiguous({0, 1, 2, 3, 4, 5}, "x");
  REQUIRE(ls->axis_labels.size() == 1);
  REQUIRE(ls->axis_labels[0] == "x");
}

TEST_CASE("contiguous::bins", "[Binning]") {
  auto ls = nuis::Binning::contiguous({0, 1, 2, 3, 4, 5}, "x");
  auto bins = ls->bins;

  REQUIRE(bins.size() == 5);
  REQUIRE(bins.front().size() == 1);
  REQUIRE(bins[0].front().low == 0);
  REQUIRE(bins[0].front().high == 1);
  REQUIRE(bins[1].front().low == 1);
  REQUIRE(bins[1].front().high == 2);
  REQUIRE(bins[4].front().low == 4);
  REQUIRE(bins[4].front().high == 5);
}

TEST_CASE("contiguous::func", "[Binning]") {
  auto ls = nuis::Binning::contiguous({0, 1, 2, 3, 4, 5}, "lx");

  REQUIRE(ls->find_bin(-0.1) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(-0) == 0);
  REQUIRE(ls->find_bin(0) == 0);
  REQUIRE(ls->find_bin(1) == 1);
  REQUIRE(ls->find_bin(4) == 4);
  REQUIRE(ls->find_bin(5) == nuis::Binning::npos);
}

TEST_CASE("contiguous::unsorted", "[Binning]") {
  REQUIRE_THROWS_AS(nuis::Binning::contiguous({3, 2, 1}, "lx"),
                    nuis::BinningUnsorted);
}

TEST_CASE("contiguous::duplicate", "[Binning]") {
  REQUIRE_THROWS_AS(nuis::Binning::contiguous({0, 1, 1, 2, 3, 4, 5}, "lx"),
                    nuis::BinningUnsorted);
}

TEST_CASE("from_extents::axis_labels", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(bins, {"x", "y", "z"});

  REQUIRE(ls->axis_labels.size() == 3);
  REQUIRE(ls->axis_labels[0] == "x");
  REQUIRE(ls->axis_labels[2] == "z");
}

TEST_CASE("from_extents::bins", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> in_bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        in_bins.emplace_back();
        in_bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        in_bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        in_bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(in_bins, {"x", "y", "z"});

  auto bins = ls->bins;

  REQUIRE(bins.size() == 27);
  REQUIRE(bins.front().size() == 3);
  REQUIRE(bins.front().front().low == 0);
  REQUIRE(bins.front().front().high == 1);
  REQUIRE(bins.front().back().low == 6);
  REQUIRE(bins.front().back().high == 7);

  REQUIRE(bins.back().size() == 3);
  REQUIRE(bins.back().front().low == 2);
  REQUIRE(bins.back().front().high == 3);
  REQUIRE(bins.back().back().low == 8);
  REQUIRE(bins.back().back().high == 9);
}

TEST_CASE("from_extents::func", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(bins, {"x", "y", "z"});

  REQUIRE(ls->find_bin({0, 3, 6}) == 0);
  REQUIRE(ls->find_bin({1, 3, 6}) == 1);
  REQUIRE(ls->find_bin({1, 3, 7}) == 10);
  REQUIRE(ls->find_bin({1, 4, 7}) == 13);
  REQUIRE(ls->find_bin({2, 5, 8}) == 26);

  // OOR above
  REQUIRE(ls->find_bin({3, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 6, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 9}) == nuis::Binning::npos);

  // OOR below
  REQUIRE(ls->find_bin({-0.1, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 2.9, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 5.9}) == nuis::Binning::npos);
}

TEST_CASE("from_extents::func -- lin_space", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t zi = 0; zi < 3; ++zi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t xi = 0; xi < 3; ++xi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(bins);
  auto ls_comp = nuis::Binning::lin_spaceND({{0, 3, 3}, {3, 6, 3}, {6, 9, 3}});

  REQUIRE(ls->find_bin({0, 3, 6}) == ls_comp->find_bin({0, 3, 6}));
  REQUIRE(ls->find_bin({1, 3, 6}) == ls_comp->find_bin({1, 3, 6}));
  REQUIRE(ls->find_bin({1, 3, 7}) == ls_comp->find_bin({1, 3, 7}));
  REQUIRE(ls->find_bin({1, 4, 7}) == ls_comp->find_bin({1, 4, 7}));
  REQUIRE(ls->find_bin({2, 5, 8}) == ls_comp->find_bin({2, 5, 8}));

  // OOR above
  REQUIRE(ls->find_bin({3, 3, 6}) == ls_comp->find_bin({3, 3, 6}));
  REQUIRE(ls->find_bin({0, 6, 6}) == ls_comp->find_bin({0, 6, 6}));
  REQUIRE(ls->find_bin({0, 3, 9}) == ls_comp->find_bin({0, 3, 9}));

  // OOR below
  REQUIRE(ls->find_bin({-0.1, 3, 6}) == ls_comp->find_bin({-0.1, 3, 6}));
  REQUIRE(ls->find_bin({0, 2.9, 6}) == ls_comp->find_bin({0, 2.9, 6}));
  REQUIRE(ls->find_bin({0, 3, 5.9}) == ls_comp->find_bin({0, 3, 5.9}));
}

TEST_CASE("from_extents::func -- reversed", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t xi = 0; xi < 3; ++xi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t zi = 0; zi < 3; ++zi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(bins, {"x", "y", "z"});

  REQUIRE(ls->find_bin({0, 3, 6}) == 0);
  REQUIRE(ls->find_bin({0, 3, 7}) == 1);
  REQUIRE(ls->find_bin({1, 3, 7}) == 10);
  REQUIRE(ls->find_bin({1, 4, 7}) == 13);
  REQUIRE(ls->find_bin({2, 5, 8}) == 26);

  // OOR above
  REQUIRE(ls->find_bin({3, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 6, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 9}) == nuis::Binning::npos);

  // OOR below
  REQUIRE(ls->find_bin({-0.1, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 2.9, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 5.9}) == nuis::Binning::npos);
}

TEST_CASE("from_extents::func -- pyramids", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  bins.push_back({{0, 1}, {0, 1}});
  bins.push_back({{1, 2}, {0, 1}});
  bins.push_back({{2, 3}, {0, 1}});
  bins.push_back({{1, 2}, {1, 2}});

  auto ls = nuis::Binning::from_extents(bins, {"x", "y"});

  REQUIRE(ls->find_bin({1.5, 1.5}) == 3);

  bins.clear();
  bins.push_back({{2, 3}, {0, 1}});
  bins.push_back({{2, 3}, {1, 2}});
  bins.push_back({{1, 2}, {1, 2}});
  bins.push_back({{2, 3}, {2, 3}});

  ls = nuis::Binning::from_extents(bins, {"x", "y"});

  REQUIRE(ls->find_bin({1.5, 1.5}) == 2);

  bins.clear();
  bins.push_back({{0, 1}, {2, 3}});
  bins.push_back({{1, 2}, {1, 2}});
  bins.push_back({{1, 2}, {2, 3}});
  bins.push_back({{2, 3}, {2, 3}});

  ls = nuis::Binning::from_extents(bins, {"x", "y"});

  REQUIRE(ls->find_bin({1.5, 1.5}) == 1);

  bins.clear();
  bins.push_back({{1, 2}, {1, 2}});
  bins.push_back({{0, 1}, {0, 1}});
  bins.push_back({{0, 1}, {1, 2}});
  bins.push_back({{0, 1}, {2, 3}});

  ls = nuis::Binning::from_extents(bins, {"x", "y"});

  REQUIRE(ls->find_bin({1.5, 1.5}) == 0);
}

TEST_CASE("from_extents::func -- sea", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins = {
      {{0.1, 0.9}}, {{1.1, 1.9}}, {{2.1, 2.9}}};

  auto ls = nuis::Binning::from_extents(bins, {
                                                  "x",
                                              });

  REQUIRE(ls->find_bin(0.5) == 0);
  REQUIRE(ls->find_bin(0) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(1) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(2) == nuis::Binning::npos);
  REQUIRE(ls->find_bin(3) == nuis::Binning::npos);

  bins.clear();
  bins = {{{0, 1}, {0, 1}},
          {{2, 3}, {0, 1}},
          {{0, 1}, {2, 3}},
          {{2, 3}, {2, 3}},
          {{1, 2}, {1, 2}}};

  ls = nuis::Binning::from_extents(bins, {"x", "y"});

  REQUIRE(ls->find_bin({1.5, 1.5}) == 4);
  REQUIRE(ls->find_bin({0.5, 1.5}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({1.5, 0.5}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({2.5, 1.5}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({1.5, 2.5}) == nuis::Binning::npos);
}

TEST_CASE("from_extents::func input vector too short", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t xi = 0; xi < 3; ++xi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t zi = 0; zi < 3; ++zi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  auto ls = nuis::Binning::from_extents(bins, {"x", "y", "z"});

  REQUIRE_THROWS_AS(ls->find_bin({0, 3}), nuis::MismatchedAxisCount);
}

TEST_CASE("from_extents::bins not unique", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t xi = 0; xi < 3; ++xi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t zi = 0; zi < 3; ++zi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  bins.emplace_back();
  bins.back().push_back({
      xmin,
      xmin + 1,
  });
  bins.back().push_back({
      ymin,
      ymin + 1,
  });
  bins.back().push_back({
      zmin,
      zmin + 1,
  });

  REQUIRE_THROWS_AS(nuis::Binning::from_extents(bins, {"x", "y", "z"}),
                    nuis::BinningNotUnique);
}
TEST_CASE("from_extents::bins with overlaps", "[Binning]") {

  std::vector<nuis::Binning::BinExtents> bins;

  double xmin = 0;
  double ymin = 3;
  double zmin = 6;
  for (size_t xi = 0; xi < 3; ++xi) {
    for (size_t yi = 0; yi < 3; ++yi) {
      for (size_t zi = 0; zi < 3; ++zi) {
        bins.emplace_back();
        bins.back().push_back({
            xmin + xi * 1,
            xmin + (xi + 1) * 1,
        });
        bins.back().push_back({
            ymin + yi * 1,
            ymin + (yi + 1) * 1,
        });
        bins.back().push_back({
            zmin + zi * 1,
            zmin + (zi + 1) * 1,
        });
      }
    }
  }

  bins.emplace_back();
  bins.back().push_back({
      xmin - 0.5,
      xmin + 0.5,
  });
  bins.back().push_back({
      ymin - 0.5,
      ymin + 0.5,
  });
  bins.back().push_back({
      zmin - 0.5,
      zmin + 0.5,
  });

  REQUIRE_THROWS_AS(nuis::Binning::from_extents(bins, {"x", "y", "z"}),
                    nuis::BinningHasOverlaps);
}

TEST_CASE("product::axis_labels", "[Binning]") {

  auto lsx = nuis::Binning::lin_space(0, 3, 3, "x");
  auto lsy = nuis::Binning::lin_space(3, 6, 3, "y");
  auto lsz = nuis::Binning::lin_space(6, 9, 3, "z");

  auto ls = nuis::Binning::product({lsx, lsy, lsz});

  REQUIRE(ls->axis_labels.size() == 3);
  REQUIRE(ls->axis_labels[0] == "x");
  REQUIRE(ls->axis_labels[2] == "z");
}

TEST_CASE("product::bins", "[Binning]") {

  auto lsx = nuis::Binning::lin_space(0, 3, 3, "x");
  auto lsy = nuis::Binning::lin_space(3, 6, 3, "y");
  auto lsz = nuis::Binning::lin_space(6, 9, 3, "z");

  auto ls = nuis::Binning::product({lsx, lsy, lsz});

  auto bins = ls->bins;

  REQUIRE(bins.size() == 27);
  REQUIRE(bins.front().size() == 3);
  REQUIRE(bins.front().front().low == 0);
  REQUIRE(bins.front().front().high == 1);
  REQUIRE(bins.front().back().low == 6);
  REQUIRE(bins.front().back().high == 7);

  REQUIRE(bins.back().size() == 3);
  REQUIRE(bins.back().front().low == 2);
  REQUIRE(bins.back().front().high == 3);
  REQUIRE(bins.back().back().low == 8);
  REQUIRE(bins.back().back().high == 9);
}

TEST_CASE("product::func", "[Binning]") {

  auto lsx = nuis::Binning::lin_space(0, 3, 3, "x");
  auto lsy = nuis::Binning::lin_space(3, 6, 3, "y");
  auto lsz = nuis::Binning::lin_space(6, 9, 3, "z");

  auto ls = nuis::Binning::product({lsx, lsy, lsz});

  REQUIRE(ls->find_bin({0, 3, 6}) == 0);
  REQUIRE(ls->find_bin({1, 3, 6}) == 1);
  REQUIRE(ls->find_bin({1, 3, 7}) == 10);
  REQUIRE(ls->find_bin({1, 4, 7}) == 13);
  REQUIRE(ls->find_bin({2, 5, 8}) == 26);

  // OOR above
  REQUIRE(ls->find_bin({3, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 6, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 9}) == nuis::Binning::npos);

  // OOR below
  REQUIRE(ls->find_bin({-0.1, 3, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 2.9, 6}) == nuis::Binning::npos);
  REQUIRE(ls->find_bin({0, 3, 5.9}) == nuis::Binning::npos);
}
