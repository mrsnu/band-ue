#pragma once

#include "Rect.generated.h"

USTRUCT(BlueprintType)
struct FRect
{
    GENERATED_BODY()

    float Left;
    float Right;
    float Top;
    float Bottom;
};