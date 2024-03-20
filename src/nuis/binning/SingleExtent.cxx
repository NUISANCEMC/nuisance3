#include "nuis/binning/SingleExtent.h"

#include "fmt/core.h"

namespace nuis {

double SingleExtent::width() const { return (high - low); }

bool SingleExtent::overlaps(SingleExtent const &other) const {
  if (other.low < low) { // overlaps if other.high > low
    return (other.high > low);
  }
  if (other.high > high) { // overlaps if other.high > low
    return (other.low < high);
  }
  return true; // other is completely inside this
}

bool SingleExtent::contains(double x) const { return (x >= low) && (x < high); }

bool operator==(SingleExtent const &a, SingleExtent const &b) {
  return (a.low == b.low) && (a.high == b.high);
}
bool operator<(SingleExtent const &a, SingleExtent const &b) {
  return (a.low != b.low) ? (a.low < b.low) : (a.high < b.high);
}
std::ostream &operator<<(std::ostream &os, nuis::SingleExtent const &sext) {
  return os << fmt::format("({:.2f} - {:.2f})", sext.low, sext.high);
}
} // namespace nuis