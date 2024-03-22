#pragma once

#ifdef NUIS_ARROW_ENABLED
#include "arrow/api.h"
#endif

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
  To operator()(int i) {

    nuis_named_log("EventFrame")::log_trace(
        "[operator({})] on ColumnValueCaster<{},{}>", i,
        column_typenum_as_string(column_type<From>::id),
        column_typenum_as_string(column_type<To>::id));
    auto v = cast_col->Value(i);
    nuis_named_log("EventFrame")::log_trace("  -- {} -> {}", v, To(v));
    return v;
  }
};

template <typename To>
std::function<To(int)> get_col_cast_to(std::shared_ptr<arrow::RecordBatch> rb,
                                       std::string const &colname) {
  auto field = rb->schema()->GetFieldByName(colname);
  if (!field) {
    throw NonExistantColumnName()
        << "[get_col_as]: No Field with name " << colname
        << " in RecordBatch.\n------\nSchema: " << *rb->schema();
  }
  auto ftype = field->type();
  if (ftype->Equals(arrow::BooleanType())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(0),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<bool, To>{
        std::dynamic_pointer_cast<typename column_type<bool>::ATT::ArrayType>(
            rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::Int64Type())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(1),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<int, To>{
        std::dynamic_pointer_cast<typename column_type<int>::ATT::ArrayType>(
            rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::UInt64Type())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(2),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<uint, To>{
        std::dynamic_pointer_cast<typename column_type<uint>::ATT::ArrayType>(
            rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::Int16Type())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(3),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<int16_t, To>{std::dynamic_pointer_cast<
        typename column_type<int16_t>::ATT::ArrayType>(
        rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::UInt16Type())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(4),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<uint16_t, To>{std::dynamic_pointer_cast<
        typename column_type<uint16_t>::ATT::ArrayType>(
        rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::FloatType())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(5),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<float, To>{
        std::dynamic_pointer_cast<typename column_type<float>::ATT::ArrayType>(
            rb->GetColumnByName(colname))};
  } else if (ftype->Equals(arrow::DoubleType())) {
    nuis_named_log("EventFrame")::log_debug(
        "[get_col_cast_to]: column {} is type {}. Producing casting functor to "
        "type {}",
        colname, column_typenum_as_string(6),
        column_typenum_as_string(column_type<To>::id));
    return ColumnValueCaster<double, To>{
        std::dynamic_pointer_cast<typename column_type<double>::ATT::ArrayType>(
            rb->GetColumnByName(colname))};
  }
  throw WrongColumnType()
      << "[get_col_as]: Column with name " << colname
      << " in RecordBatch is of unhandled type.\n------\nSchema: "
      << *rb->schema();
}

template <typename RT>
inline std::shared_ptr<typename column_type<RT>::ATT::ArrayType>
get_col_as(std::shared_ptr<arrow::RecordBatch> rb, std::string const &colname) {
  auto gcolptr = rb->GetColumnByName(colname);
  if (!gcolptr) {
    throw NonExistantColumnName()
        << "[get_col_as]: No Column with name " << colname
        << " in RecordBatch.\n------\nSchema: " << *rb->schema();
  }
  auto tcolptr =
      std::dynamic_pointer_cast<typename column_type<RT>::ATT::ArrayType>(
          gcolptr);
  if (!tcolptr) {
    throw WrongColumnType() << "[get_col_as]: Column with name " << colname
                            << " in RecordBatch is not of type: "
                            << column_typenum_as_string(column_type<RT>::id)
                            << ".\n------\nSchema: " << *rb->schema();
  }
  return tcolptr;
}
#endif

} // namespace nuis