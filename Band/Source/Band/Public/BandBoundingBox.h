// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BandLandmark.h"
#include "UObject/ObjectMacros.h"
#include "Rect.h"
#include "BandBoundingBox.generated.h"

USTRUCT(BlueprintType)
struct BAND_API FBandBoundingBox {
  GENERATED_BODY()

  FBandBoundingBox() = default;
  FBandBoundingBox(float Confidence, FRect Position, FString Label = TEXT(""),
                   TArray<FBandLandmark> Landmark = {});

  bool operator==(const FBandBoundingBox& rhs) const {
    return (Confidence == rhs.Confidence) && (Position == rhs.Position) && (
             Label == rhs.Label) && (Landmark == rhs.Landmark);
  }

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float Confidence;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FRect Position;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FString Label;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  TArray<FBandLandmark> Landmark;
};

inline bool operator<(const FBandBoundingBox& Lhs,
                      const FBandBoundingBox& Rhs) {
  return Lhs.Confidence > Rhs.Confidence;
}
