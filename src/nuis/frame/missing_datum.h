#pragma once

#include <limits>

namespace nuis {

template <typename T>
inline constexpr T kMissingDatum = std::numeric_limits<T>::max();
template <> inline constexpr double kMissingDatum<double> = 0xdeadbeef;
template <>
inline constexpr int kMissingDatum<int> = std::numeric_limits<int>::max();

} // namespace nuis