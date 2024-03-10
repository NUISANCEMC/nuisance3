#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "nuis/frame/Frame.h"

#include <cassert>

TEST_CASE("Frame::find_column_index", "[Frame]") {
  nuis::Frame f;
  f.column_names.push_back("a");
  f.column_names.push_back("b");
  f.column_names.push_back("c");
  f.table = Eigen::ArrayXXd::Zero(0, 3);

  REQUIRE(f.find_column_index("a") == 0);
  REQUIRE(f.find_column_index("b") == 1);
  REQUIRE(f.find_column_index("c") == 2);
  REQUIRE(f.find_column_index("d") == nuis::Frame::npos);
}

TEST_CASE("Frame::col(string)", "[Frame]") {
  nuis::Frame f;
  f.column_names.push_back("a");
  f.column_names.push_back("b");
  f.column_names.push_back("c");
  f.table = Eigen::ArrayXXd::Zero(0, 3);

  REQUIRE_NOTHROW(f.col("a"));
  REQUIRE_NOTHROW(f.col("b"));
  REQUIRE_NOTHROW(f.col("c"));
  REQUIRE_THROWS_AS(f.col("d"), nuis::InvalidFrameColumnName);
}

TEST_CASE("Set Eigen::Ref from Frame::col(string)", "[Frame]") {
  nuis::Frame f;
  f.column_names.push_back("a");
  f.column_names.push_back("b");
  f.column_names.push_back("c");
  f.table = Eigen::ArrayXXd::Zero(3, 3);
  f.table.col(0) = Eigen::ArrayXd::Constant(3, 111);
  f.table.col(1) = Eigen::ArrayXd::Constant(3, 222);
  f.table.col(2) = Eigen::ArrayXd::Constant(3, 333);

  REQUIRE(f.col("a")[0] == 111);
  REQUIRE(f.col("b")[1] == 222);
  REQUIRE(f.col("c")[2] == 333);

  REQUIRE(f.col("c")[0] == 333);
  f.col("c")[0] = -123;
  REQUIRE(f.col("c")[0] == -123);
}

TEST_CASE("Set Eigen::Ref from Frame::cols(strings)", "[Frame]") {
  nuis::Frame f;
  f.column_names.push_back("a");
  f.column_names.push_back("b");
  f.column_names.push_back("c");
  f.table = Eigen::ArrayXXd::Zero(3, 3);
  f.table.col(0) = Eigen::ArrayXd::Constant(3, 111);
  f.table.col(1) = Eigen::ArrayXd::Constant(3, 222);
  f.table.col(2) = Eigen::ArrayXd::Constant(3, 333);

  REQUIRE(f.cols({"a", "b"})[0][0] == 111);
  REQUIRE(f.cols({"a", "b"})[1][1] == 222);
  REQUIRE(f.cols({"c"})[0][0] == 333);
  REQUIRE_THROWS_AS(f.cols({"d"}), nuis::InvalidFrameColumnName);

  f.cols({"a", "b", "c"})[0][1] = -123;
  f.cols({"a", "b", "c"})[1][1] = -123;
  f.cols({"a", "b", "c"})[2][1] = -123;
  REQUIRE(f.cols({"a"})[0][1] == -123);
  REQUIRE(f.cols({"b"})[0][1] == -123);
  REQUIRE(f.cols({"c"})[0][1] == -123);
}
