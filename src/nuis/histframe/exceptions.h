#pragma once

#include "nuis/except.h"

namespace nuis {
DECLARE_NUISANCE_EXCEPT(MissingProjectionEncountered);
DECLARE_NUISANCE_EXCEPT(InvalidColumnAccess);
DECLARE_NUISANCE_EXCEPT(InvalidColumnName);
DECLARE_NUISANCE_EXCEPT(InvalidOperation);
} // namespace nuis