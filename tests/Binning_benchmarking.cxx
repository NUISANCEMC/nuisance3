#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"

#include "nuis/binning/Binning.h"
#include "nuis/binning/exceptions.h"
#include "nuis/binning/utility.h"
#include "nuis/log.txx"

#include "TAxis.h"

#include <cassert>
#include <random>

TEST_CASE("lin_space", "[Binning]") {
  auto lin_bins = nuis::Binning::lin_space(-10, 10, 100);

  std::shared_ptr<TAxis> rootx = std::make_shared<TAxis>(100, -10, 10);

  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_real_distribution<> uni(-10, 10);

  size_t ntest = 1E6;

  std::vector<double> rvalsx(ntest);
  std::vector<double> rvalsy(ntest);
  std::vector<double> rvalsz(ntest);
  for (size_t i = 0; i < ntest; ++i) {
    rvalsx[i] = uni(e1);
    rvalsy[i] = uni(e1);
    rvalsz[i] = uni(e1);
  }

  BENCHMARK("[ROOT] lin_space:1D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = rootx->FindBin(rvalsx[i]);
    }
    return bin;
  };

  BENCHMARK("[nuis] lin_space:1D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins->find_bin(rvalsx[i]);
    }
    return bin;
  };

  for (size_t i = 0; i < std::min(ntest, 1000ul); ++i) {
    REQUIRE(int(lin_bins->find_bin(rvalsx[i]) + 1) ==
            rootx->FindBin(rvalsx[i]));
  }

  auto lin_bins1d = nuis::Binning::lin_spaceND({{-10, 10, 100}});

  BENCHMARK("[nuis] lin_spaceND:1D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins1d->find_bin({rvalsx[i]});
    }
    return bin;
  };

  auto lin_bins2d =
      nuis::Binning::lin_spaceND({{-10, 10, 100}, {-10, 10, 100}});

  BENCHMARK("[nuis] lin_spaceND:2D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins2d->find_bin({rvalsx[i], rvalsy[i]});
    }
    return bin;
  };

  auto lin_bins3d = nuis::Binning::lin_spaceND(
      {{-10, 10, 100}, {-10, 10, 100}, {-10, 10, 100}});

  BENCHMARK("[nuis] lin_spaceND:3D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins3d->find_bin({rvalsx[i], rvalsy[i], rvalsz[i]});
    }
    return bin;
  };

  auto prod_lin_bins2d = nuis::Binning::product({lin_bins, lin_bins});

  BENCHMARK("[nuis] product(lin):2D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = prod_lin_bins2d->find_bin({rvalsx[i], rvalsy[i]});
    }
    return bin;
  };

  auto prod_lin_bins3d = nuis::Binning::product({lin_bins, lin_bins, lin_bins});

  BENCHMARK("[nuis] product(lin):3D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = prod_lin_bins3d->find_bin({rvalsx[i], rvalsy[i], rvalsz[i]});
    }
    return bin;
  };
}

TEST_CASE("contiguous", "[Binning]") {
  auto lin_edges = nuis::lin_spaced_edges(-10, 10, 100);

  auto lin_bins = nuis::Binning::contiguous(lin_edges);

  std::shared_ptr<TAxis> rootx =
      std::make_shared<TAxis>(lin_edges.size() - 1, lin_edges.data());

  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_real_distribution<> uni(-10, 10);

  size_t ntest = 1E6;

  std::vector<double> rvals(ntest);
  for (size_t i = 0; i < ntest; ++i) {
    rvals[i] = uni(e1);
  }

  BENCHMARK("[ROOT] contiguous:1D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = rootx->FindBin(rvals[i]);
    }
    return bin;
  };

  BENCHMARK("[nuis] contiguous:1D, n = 1E6") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins->find_bin(rvals[i]);
    }
    return bin;
  };

  for (size_t i = 0; i < std::min(ntest, 1000ul); ++i) {
    REQUIRE(int(lin_bins->find_bin(rvals[i]) + 1) == rootx->FindBin(rvals[i]));
  }
}

TEST_CASE("from_extents", "[Binning]") {
  auto lin_exts = nuis::edges_to_extents(nuis::lin_spaced_edges(-10, 10, 20));
  auto lin_exts_long = nuis::edges_to_extents(nuis::lin_spaced_edges(-10, 10, 40));

  auto lin_bins = nuis::Binning::from_extents(lin_exts);
  auto lin_bins_long = nuis::Binning::from_extents(lin_exts_long);

  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_real_distribution<> uni(-10, 10);

  size_t ntest = 1E5;

  std::vector<double> rvalsx(ntest);
  std::vector<double> rvalsy(ntest);
  std::vector<double> rvalsz(ntest);
  for (size_t i = 0; i < ntest; ++i) {
    rvalsx[i] = uni(e1);
    rvalsy[i] = uni(e1);
    rvalsz[i] = uni(e1);
  }

  BENCHMARK("[nuis] from_extents:1D, n = 1E5") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins->find_bin(rvalsx[i]);
    }
    return bin;
  };

  auto lin_bins2D = nuis::Binning::from_extents(
      nuis::Binning::product({lin_bins, lin_bins})->bins);

  BENCHMARK("[nuis] from_extents:2D, n = 1E5") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins2D->find_bin({rvalsx[i], rvalsy[i]});
    }
    return bin;
  };

  auto lin_bins3D = nuis::Binning::from_extents(
      nuis::Binning::product({lin_bins, lin_bins, lin_bins_long})->bins);

  BENCHMARK("[nuis] from_extents:3D, n = 1E5") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins3D->find_bin({rvalsx[i], rvalsy[i], rvalsz[i]});
    }
    return bin;
  };

  auto lin_bins3Dbf = nuis::Binning::brute_force(
      nuis::Binning::product({lin_bins, lin_bins, lin_bins_long})->bins);

  BENCHMARK("[nuis] brute_force:3D, n = 1E5") {
    int bin = 0;
    for (size_t i = 0; i < ntest; ++i) {
      bin = lin_bins3Dbf->find_bin({rvalsx[i], rvalsy[i], rvalsz[i]});
    }
    return bin;
  };
}