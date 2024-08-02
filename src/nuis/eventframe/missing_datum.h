#pragma once

#include "ProSelecta/missing_datum.h"

namespace nuis {

template <typename T> inline constexpr T kMissingDatum = ps::kMissingDatum<T>;

} // namespace nuis