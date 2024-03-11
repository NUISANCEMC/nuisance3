#pragma once

#include "nuis/except.h"

namespace nuis {
NEW_NUISANCE_EXCEPT(BinningNotIncreasing);
NEW_NUISANCE_EXCEPT(UnbinnableNumber);
NEW_NUISANCE_EXCEPT(InvalidBinEdgeForLogarithmicBinning);
NEW_NUISANCE_EXCEPT(TooFewProjectionsForBinning);
NEW_NUISANCE_EXCEPT(BinningUnsorted);
NEW_NUISANCE_EXCEPT(TooFewBinEdges);
NEW_NUISANCE_EXCEPT(BinningHasOverlaps);
NEW_NUISANCE_EXCEPT(BinningNotUnique);
NEW_NUISANCE_EXCEPT(MismatchedAxisCount);
NEW_NUISANCE_EXCEPT(AxisOverflow);
NEW_NUISANCE_EXCEPT(CatastrophicBinningFailure);
NEW_NUISANCE_EXCEPT(EmptyBinning);
} // namespace nuis