#pragma once

#include <functional>

namespace HepMC3 {
  class GenEvent;
}

namespace nuis {
namespace weight {
using func = std::function<double(HepMC3::GenEvent const &)>;

} // namespace weight
} // namespace nuis