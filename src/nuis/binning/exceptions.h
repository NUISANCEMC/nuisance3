#pragma once

#include "nuis/except.h"

namespace nuis {
DECLARE_NUISANCE_EXCEPT(BinningNotIncreasing);
DECLARE_NUISANCE_EXCEPT(UnbinnableNumber);
DECLARE_NUISANCE_EXCEPT(InvalidBinEdgeForLogarithmicBinning);
DECLARE_NUISANCE_EXCEPT(TooFewProjectionsForBinning);
DECLARE_NUISANCE_EXCEPT(BinningUnsorted);
DECLARE_NUISANCE_EXCEPT(TooFewBinEdges);
DECLARE_NUISANCE_EXCEPT(BinningHasOverlaps);
DECLARE_NUISANCE_EXCEPT(BinningNotUnique);
DECLARE_NUISANCE_EXCEPT(MismatchedAxisCount);
DECLARE_NUISANCE_EXCEPT(AxisOverflow);
DECLARE_NUISANCE_EXCEPT(CatastrophicBinningFailure);
DECLARE_NUISANCE_EXCEPT(EmptyBinning);
DECLARE_NUISANCE_EXCEPT(NonContiguousBinning);
} // namespace nuis