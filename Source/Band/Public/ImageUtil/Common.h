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
  lhs = rexpr;

#define COMMON_MACROS_IMPL_CONCAT_INNER_(x, y) x##y
#define COMMON_MACROS_IMPL_CONCAT_(x, y) COMMON_MACROS_IMPL_CONCAT_INNER_(x, y)

// Internal helpers for macro expansion.
#define COMMON_MACROS_IMPL_EAT(...)
#define COMMON_MACROS_IMPL_REM(...) __VA_ARGS__
#define COMMON_MACROS_IMPL_EMPTY()

// Internal helpers for emptyness arguments check.
#define COMMON_MACROS_IMPL_IS_EMPTY_INNER(...) \
  COMMON_MACROS_IMPL_IS_EMPTY_INNER_I(__VA_ARGS__, 0, 1)
#define COMMON_MACROS_IMPL_IS_EMPTY_INNER_I(e0, e1, is_empty, ...) is_empty

#define COMMON_MACROS_IMPL_IS_EMPTY(...) \
  COMMON_MACROS_IMPL_IS_EMPTY_I(__VA_ARGS__)
#define COMMON_MACROS_IMPL_IS_EMPTY_I(...) \
  COMMON_MACROS_IMPL_IS_EMPTY_INNER(_, ##__VA_ARGS__)

// Internal helpers for if statement.
#define COMMON_MACROS_IMPL_IF_1(_Then, _Else) _Then
#define COMMON_MACROS_IMPL_IF_0(_Then, _Else) _Else
#define COMMON_MACROS_IMPL_IF(_Cond, _Then, _Else)          \
  COMMON_MACROS_IMPL_CONCAT_(COMMON_MACROS_IMPL_IF_, _Cond) \
  (_Then, _Else)

namespace band {
struct BoundingBox {
  int32_t origin_x;
  int32_t origin_y;
  int32_t width;
  int32_t height;
};
}
