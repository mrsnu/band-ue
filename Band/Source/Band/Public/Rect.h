#pragma once

#include "Rect.generated.h"

USTRUCT(BlueprintType)
struct FRect {
	GENERATED_BODY()

	float Left;
	float Right;
	float Top;
	float Bottom;
};

inline float BoxArea(FRect A) { return (A.Right - A.Left) * (A.Top - A.Bottom); }

inline float BoxIntersection(FRect A, FRect B) {
	if (A.Right <= B.Left || B.Right <= A.Left)
		return 0.0f;
	if (A.Top <= B.Bottom || B.Top <= A.Bottom)
		return 0.0f;
	return (FGenericPlatformMath::Min(A.Right, B.Right) - FGenericPlatformMath::Max(A.Left, B.Left))
		* (FGenericPlatformMath::Min(A.Top, B.Top) - FGenericPlatformMath::Max(A.Bottom, B.Bottom));
}


inline float BoxUnion(FRect A, FRect B) { return BoxArea(A) + BoxArea(B) - BoxIntersection(A, B); }

inline float BoxIou(FRect A, FRect B) { return BoxIntersection(A, B) / BoxUnion(A, B); }
