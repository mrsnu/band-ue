#pragma once

#include "Core.h"

#define RETURN_IF_ERROR(Value) \
  do                           \
  {                            \
    if (!Value)                \
      return false;            \
  } while (0)

#define ASSIGN_OR_RETURN(lhs, rexpr) \
  do                                 \
  {                                  \
    auto statusor = (rexpr);         \
    if (!statusor)                   \
    {                                \
      return false;                  \
    }                                \
  } while (0);                        \
  lhs = std::move(rexpr);

#define STATUS_MACROS_IMPL_CONCAT_INNER_(x, y) x##y
#define STATUS_MACROS_IMPL_CONCAT_(x, y) STATUS_MACROS_IMPL_CONCAT_INNER_(x, y)

// Internal helpers for macro expansion.
#define STATUS_MACROS_IMPL_EAT(...)
#define STATUS_MACROS_IMPL_REM(...) __VA_ARGS__
#define STATUS_MACROS_IMPL_EMPTY()

// Internal helpers for emptyness arguments check.
#define STATUS_MACROS_IMPL_IS_EMPTY_INNER(...) \
  STATUS_MACROS_IMPL_IS_EMPTY_INNER_I(__VA_ARGS__, 0, 1)
#define STATUS_MACROS_IMPL_IS_EMPTY_INNER_I(e0, e1, is_empty, ...) is_empty

#define STATUS_MACROS_IMPL_IS_EMPTY(...) \
  STATUS_MACROS_IMPL_IS_EMPTY_I(__VA_ARGS__)
#define STATUS_MACROS_IMPL_IS_EMPTY_I(...) \
  STATUS_MACROS_IMPL_IS_EMPTY_INNER(_, ##__VA_ARGS__)

// Internal helpers for if statement.
#define STATUS_MACROS_IMPL_IF_1(_Then, _Else) _Then
#define STATUS_MACROS_IMPL_IF_0(_Then, _Else) _Else
#define STATUS_MACROS_IMPL_IF(_Cond, _Then, _Else)          \
  STATUS_MACROS_IMPL_CONCAT_(STATUS_MACROS_IMPL_IF_, _Cond) \
  (_Then, _Else)

namespace Band
{

  struct BoundingBox
  {
    int32_t origin_x;
    int32_t origin_y;
    int32_t width;
    int32_t height;
  };

}
