#include "nuis/histframe/Binning.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

using namespace nuis;

int main(int argc, char const *argv[]) {

  auto linbin = Binning::lin_space(0, 10, 100);

  std::cout << linbin << std::endl;
  std::cout << "find_bin(1) = " << linbin(1) << std::endl;
}