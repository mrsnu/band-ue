#pragma once

#include "Rect.generated.h"

USTRUCT(BlueprintType)
struct FRect {
  GENERATED_BODY()

  const float Width() const { return Right - Left; }
  const float Height() const { return Bottom - Top; }

  bool operator==(const FRect& Rhs) const {
    return Left == Rhs.Left &&
           Right == Rhs.Right &&
           Top == Rhs.Top &&
           Bottom == Rhs.Bottom;
  }

  // Returns the intersection of two rectangles.
  FRect operator+(const FRect& Rhs) const {
    return {FGenericPlatformMath::Min(Left, Rhs.Left),
            FGenericPlatformMath::Max(Right, Rhs.Right),
            FGenericPlatformMath::Min(Top, Rhs.Top),
            FGenericPlatformMath::Max(Bottom, Rhs.Bottom)};
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

inline float BoxArea(const FRect& Rect) {
  return (Rect.Right - Rect.Left) * (Rect.Bottom - Rect.Top);
}

inline float BoxIntersection(const FRect& Lhs, const FRect& Rhs) {
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


inline float BoxUnion(const FRect& Lhs, const FRect& Rhs) {
  return BoxArea(Lhs) + BoxArea(Rhs) - BoxIntersection(Lhs, Rhs);
}

inline float BoxIou(const FRect& Lhs, const FRect& Rhs) {
  return BoxIntersection(Lhs, Rhs) / BoxUnion(Lhs, Rhs);
}
