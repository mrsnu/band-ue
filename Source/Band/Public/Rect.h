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

  UPROPERTY()
  float Left;
  UPROPERTY()
  float Right;
  UPROPERTY()
  float Top;
  UPROPERTY()
  float Bottom;
};

inline float BoxArea(FRect Rect) {
  return (Rect.Right - Rect.Left) * (Rect.Bottom - Rect.Top);
}

inline float BoxIntersection(FRect Lhs, FRect Rhs) {
  if (Lhs.Right <= Rhs.Left || Rhs.Right <= Lhs.Left) {
    return 0.0f;
  }
  if (Lhs.Bottom <= Rhs.Top || Rhs.Bottom <= Lhs.Top) {
    return 0.0f;
  }
  return (FGenericPlatformMath::Min(Lhs.Right, Rhs.Right) -
          FGenericPlatformMath::Max(Lhs.Left, Rhs.Left)) *
         (FGenericPlatformMath::Min(Lhs.Bottom, Rhs.Bottom) -
          FGenericPlatformMath::Max(Lhs.Top, Rhs.Top));
}


inline float BoxUnion(FRect Lhs, FRect Rhs) {
  return BoxArea(Lhs) + BoxArea(Rhs) - BoxIntersection(Lhs, Rhs);
}

inline float BoxIou(FRect Lhs, FRect Rhs) {
  return BoxIntersection(Lhs, Rhs) / BoxUnion(Lhs, Rhs);
}
