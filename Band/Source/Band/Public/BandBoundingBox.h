// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

#include "BandLandmark.h"
#include "UObject/ObjectMacros.h"
#include "Rect.h"
#include "BandBoundingBox.generated.h"

USTRUCT(BlueprintType)
struct BAND_API FBandBoundingBox {
  GENERATED_BODY()

  FBandBoundingBox() = default;
  FBandBoundingBox(float Confidence, FRect Position, FString Label = TEXT(""),
                   TArray<FBandLandmark> Landmark = {},
                   TArray<FIntPoint> LandmarkEdge = {});

  bool operator==(const FBandBoundingBox& Rhs) const;
  FString ToString() const;

  float GetWidth() const;
  float GetHeight() const;
  float GetCenterX() const;
  float GetCenterY() const;
  float GetSimilarity(const FBandBoundingBox& Rhs) const;

  static std::function<bool(const FBandBoundingBox&, const FBandBoundingBox&)>
  GetPositionBasedComparator() {
    return [](const FBandBoundingBox& Lhs, const FBandBoundingBox& Rhs) {
      return Lhs.Position.Left < Rhs.Position.Left;
    };
  }

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float Confidence;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FRect Position;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FString Label;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  TArray<FBandLandmark> Landmark;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  TArray<FIntPoint> LandmarkEdge;
};

inline bool operator<(const FBandBoundingBox& Lhs,
                      const FBandBoundingBox& Rhs) {
  return Lhs.Confidence > Rhs.Confidence;
}
