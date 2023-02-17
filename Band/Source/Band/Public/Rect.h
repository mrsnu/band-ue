#pragma once

#include "Rect.generated.h"

USTRUCT(BlueprintType)
struct FRect {
  GENERATED_BODY()

  bool operator==(const FRect& Rhs) const {
    return Left == Rhs.Left &&
           Right == Rhs.Right &&
           Top == Rhs.Top &&
           Bottom == Rhs.Bottom;
  }

  float Left;
  float Right;
  float Top;
  float Bottom;
};

inline float BoxArea(FRect Rect) {
  return (Rect.Right - Rect.Left) * (Rect.Top - Rect.Bottom);
}

inline float BoxIntersection(FRect Lhs, FRect Rhs) {
  if (Lhs.Right <= Rhs.Left || Rhs.Right <= Lhs.Left) {
    return 0.0f;
  }
  if (Lhs.Top <= Rhs.Bottom || Rhs.Top <= Lhs.Bottom) {
    return 0.0f;
  }
  return (FGenericPlatformMath::Min(Lhs.Right, Rhs.Right) -
          FGenericPlatformMath::Max(Lhs.Left, Rhs.Left))
         * (FGenericPlatformMath::Min(Lhs.Top, Rhs.Top) -
            FGenericPlatformMath::Max(
                Lhs.Bottom, Rhs.Bottom));
}


inline float BoxUnion(FRect Lhs, FRect Rhs) {
  return BoxArea(Lhs) + BoxArea(Rhs) - BoxIntersection(Lhs, Rhs);
}

inline float BoxIou(FRect Lhs, FRect Rhs) {
  return BoxIntersection(Lhs, Rhs) / BoxUnion(Lhs, Rhs);
}
