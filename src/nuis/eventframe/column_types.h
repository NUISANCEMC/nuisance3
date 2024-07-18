#pragma once

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

#include "nuis/except.h"

#include <iostream>

#include <cstdint>

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
struct LoudDeleter {
  void operator()(arrow::ArrayBuilder *o) {
    std::cout << "deleting: " << o << std::endl;
    std::default_delete<arrow::ArrayBuilder>()(o);
  }
};

using ArrowBuilderPtr = std::unique_ptr<arrow::ArrayBuilder>;

#define NUIS_COLUMN_TYPE(ctype, atype, typenum)                                \
  template <> struct column_type<ctype> {                                      \
    constexpr static int id = typenum;                                         \
    using CT = ctype;                                                          \
    using AT = atype;                                                          \
    using ATT = arrow::TypeTraits<AT>;                                         \
    static std::shared_ptr<AT> mkt() { return std::make_shared<AT>(); }        \
    static ArrowBuilderPtr mkb() {                                             \
      return std::unique_ptr<ATT::BuilderType>(new ATT::BuilderType());        \
    }                                                                          \
  };
#else
#define NUIS_COLUMN_TYPE(ctype, atype, typenum)                                \
  template <> struct column_type<ctype> {                                      \
    constexpr static int id = typenum;                                         \
    using CT = ctype;                                                          \
  };
#endif

NUIS_COLUMN_TYPE(bool, arrow::BooleanType, 0);
NUIS_COLUMN_TYPE(int, arrow::Int64Type, 1);
NUIS_COLUMN_TYPE(uint, arrow::UInt64Type, 2);
NUIS_COLUMN_TYPE(int16_t, arrow::Int16Type, 3);
NUIS_COLUMN_TYPE(uint16_t, arrow::UInt16Type, 4);
NUIS_COLUMN_TYPE(float, arrow::FloatType, 5);
NUIS_COLUMN_TYPE(double, arrow::DoubleType, 6);

inline std::string column_typenum_as_string(int i) {
  switch (i) {
  case 0:
    return "bool";
  case 1:
    return "int";
  case 2:
    return "uint";
  case 3:
    return "int16";
  case 4:
    return "uint16";
  case 5:
    return "float";
  case 6:
    return "double";
  }
  return "";
}

#ifdef NUIS_ARROW_ENABLED

NEW_NUISANCE_EXCEPT(NonExistantColumnName);
NEW_NUISANCE_EXCEPT(WrongColumnType);

template <typename From, typename To> struct ColumnValueCaster {
  std::shared_ptr<typename column_type<From>::ATT::ArrayType> cast_col;
  To operator()(int i);
};

template <typename To>
std::function<To(int)> get_col_cast_to(std::shared_ptr<arrow::RecordBatch> rb,
                                       std::string const &colname);
template <typename RT>
std::shared_ptr<typename column_type<RT>::ATT::ArrayType>
get_col_as(std::shared_ptr<arrow::RecordBatch> rb, std::string const &colname);

#endif

} // namespace nuis
