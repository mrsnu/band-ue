// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"
#include "Rect.h"
#include "BandBoundingBox.generated.h"

USTRUCT(BlueprintType)
struct BAND_API FBandBoundingBox
{
	GENERATED_BODY()

	FBandBoundingBox() = default;
	FBandBoundingBox(float Confidence, FRect Position, FString Label = TEXT(""), TArray<FVector> Landmark = {});

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Confidence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRect Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector> Landmark;
};

inline bool operator<(const FBandBoundingBox& Lhs, const FBandBoundingBox& Rhs)
{
	return Lhs.Confidence > Rhs.Confidence;
}
