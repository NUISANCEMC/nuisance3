#pragma once

#include "nuis/histframe/HistFrame.h"

#include <string>
#include <utility>

namespace nuis {
struct Comparison {

  std::string normalisation_type;
  bool by_bin_width;

  Eigen::ArrayXXd correlation;

  HistFrame mc;
  HistFrame data;

  Comparison(BinningPtr bindef) {
    mc = HistFrame(bindef, "mc");
    data = HistFrame(bindef, "data");
  }

  // Redirect most function calls down to mc as data is static
  template <typename... TS> auto add_column(TS &&...args) {
    return mc.add_column(std::forward<TS>(args)...);
  }

  template <typename... TS> auto find_column_index(TS &&...args) {
    return mc.find_column_index(std::forward<TS>(args)...);
  }

  template <typename... TS> auto get_values(TS &&...args) {
    return mc.get_values(std::forward<TS>(args)...);
  }

  template <typename... TS> auto get_errors(TS &&...args) {
    return mc.get_errors(std::forward<TS>(args)...);
  }

  template <typename... TS> auto get_column(TS &&...args) {
    return mc.get_column(std::forward<TS>(args)...);
  }

  template <typename... TS> void fill(TS &&...args) {
    return mc.fill(std::forward<TS>(args)...);
  }

  template <typename... TS> void fill_with_selection(TS &&...args) {
    return mc.fill_with_selection(std::forward<TS>(args)...);
  }

  template <typename... TS> auto find_bin(TS &&...args) {
    return mc.find_bin(std::forward<TS>(args)...);
  }

  void reset() { return mc.reset(); }

  template <typename... TS> auto get_data_values(TS &&...args) {
    return data.get_values(std::forward<TS>(args)...);
  }

  template <typename... TS> auto get_data_errors(TS &&...args) {
    return data.get_errors(std::forward<TS>(args)...);
  }

  // LP: Can't remember what we decided about this, but I don't love it maybe
  // these just forward to the mc, and we say you have to use the get_data_xxx
  // functions above to get data stuff
  auto operator[](std::string const &name) {
    if (name == "data")
      return data[0];
    return mc[name];
  }

  // this also works
  constexpr static HistFrame::column_t kData = HistFrame::npos - 1;

  // Here DATA will still need its own assigned ID :(
  auto operator[](HistFrame::column_t const &colid) {
    if (colid == kData) {
      return data[0];
    }
    return mc[colid];
  }
};
} // namespace nuis
