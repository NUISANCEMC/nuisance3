#pragma once

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

namespace nuis {
template <typename T> struct column_type {
  constexpr static int id = 0;
  using CT = void;
#ifdef NUIS_ARROW_ENABLED
  using AT = void;
  using ATT = arrow::TypeTraits<AT>;
#endif
};

#ifdef NUIS_ARROW_ENABLED
#define NUIS_COLUMN_TYPE(ctype, atype, typenum)                                \
  template <> struct column_type<ctype> {                                      \
    constexpr static int id = typenum;                                         \
    using CT = ctype;                                                          \
    using AT = atype;                                                          \
    using ATT = arrow::TypeTraits<AT>;                                         \
    static std::shared_ptr<AT> mkt() { return std::make_shared<AT>(); }        \
    static std::unique_ptr<ATT::BuilderType> mkb() {                           \
      return std::make_unique<ATT::BuilderType>();                             \
    }                                                                          \
  };
#else
#define NUIS_COLUMN_TYPE(ctype, atype, typenum)                                \
  template <> struct column_type<ctype> {                                      \
    constexpr static int id = typenum;                                         \
    using CT = ctype;                                                          \
  };
#endif

NUIS_COLUMN_TYPE(int, arrow::Int64Type, 1);
NUIS_COLUMN_TYPE(float, arrow::FloatType, 2);
NUIS_COLUMN_TYPE(double, arrow::DoubleType, 3);

inline std::string column_typenum_as_string(int i) {
  switch (i) {
  case 1:
    return "int";
  case 2:
    return "float";
  case 3:
    return "double";
  }
  return "";
}

} // namespace nuis