#include "nuis/eventframe/column_types.h"

#include "nuis/log.txx"

#define COLUMN_TYPE_ITER                                                       \
  X(bool)                                                                      \
  X(int)                                                                       \
  X(uint)                                                                      \
  X(int16_t)                                                                   \
  X(uint16_t)                                                                  \
  X(float)                                                                     \
  X(double)

#ifdef NUIS_ARROW_ENABLED
namespace nuis {

template <typename From, typename To>
To ColumnValueCaster<From, To>::operator()(int i) {

  NUIS_LOGGER_TRACE("EventFrame", "[operator({})] on ColumnValueCaster<{},{}>",
                    i, column_typenum_as_string(column_type<From>::id),
                    column_typenum_as_string(column_type<To>::id));

  auto v = cast_col->Value(i);

  NUIS_LOGGER_TRACE("EventFrame", "  -- {} -> {}", v, To(v));

  return v;
}

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

#define X(t)                                                                   \
  if (ftype->Equals(typename column_type<t>::AT())) {                          \
    nuis_named_log("EventFrame")::log_debug(                                   \
        "[get_col_cast_to]: column {} is type {}. Producing casting functor "  \
        "to "                                                                  \
        "type {}",                                                             \
        colname, column_typenum_as_string(column_type<t>::id),                 \
        column_typenum_as_string(column_type<To>::id));                        \
    return ColumnValueCaster<t, To>{                                           \
        std::dynamic_pointer_cast<typename column_type<t>::ATT::ArrayType>(    \
            rb->GetColumnByName(colname))};                                    \
  }

  COLUMN_TYPE_ITER

  throw WrongColumnType()
      << "[get_col_cast_to]: Column with name " << colname
      << " in RecordBatch is of unhandled type.\n------\nSchema: "
      << *rb->schema();

#undef X
}

#define X(t)                                                                   \
  template std::function<t(int)> get_col_cast_to<t>(                           \
      std::shared_ptr<arrow::RecordBatch> rb, std::string const &colname);

COLUMN_TYPE_ITER

#undef X

template <typename RT>
std::shared_ptr<typename column_type<RT>::ATT::ArrayType>
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

#define X(t)                                                                   \
  template std::shared_ptr<column_type<t>::ATT::ArrayType> get_col_as<t>(      \
      std::shared_ptr<arrow::RecordBatch> rb, std::string const &colname);

COLUMN_TYPE_ITER

#undef X

} // namespace nuis

#endif