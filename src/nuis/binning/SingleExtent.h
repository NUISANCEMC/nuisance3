#pragma once

#include <ostream>

namespace nuis {

struct SingleExtent {
  double low;
  double high;
  SingleExtent(double min = 0, double max = 0) : low(min), high(max) {}

  double width() const;
  bool overlaps(SingleExtent const &other) const;
  bool contains(double x) const;
};

bool operator<(SingleExtent const &, SingleExtent const &);
bool operator==(SingleExtent const &, SingleExtent const &);

std::ostream &operator<<(std::ostream &os, nuis::SingleExtent const &);

} // namespace nuis