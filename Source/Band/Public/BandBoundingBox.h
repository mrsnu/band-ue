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
  FBandBoundingBox(float Confidence,
                   FRect Position,
                   FString Label = TEXT(""),
                   float Displacement = 0.f,
                   // Default to have low priority
                   TArray<FBandLandmark> Landmark = {},
                   TArray<FIntPoint> LandmarkEdge = {});

  bool operator==(const FBandBoundingBox& Rhs) const;
  FString ToString() const;

  FVector GetSize() const;
  FVector GetCenter() const;
  FVector GetOrigin() const;

  float GetDiagonal() const;
  float GetWidth() const;
  float GetHeight() const;
  float GetCenterX() const;
  float GetCenterY() const;
  float GetSimilarity(const FBandBoundingBox& Rhs) const;
  float GetDistance(const FBandBoundingBox& Rhs) const;
  float GetLandmarkDistance(const FBandBoundingBox& Rhs) const;

  static std::function<bool(const FBandBoundingBox&, const FBandBoundingBox&)>
  GetPositionBasedComparator() {
    return [](const FBandBoundingBox& Lhs, const FBandBoundingBox& Rhs) {
      return Lhs.Position.Left < Rhs.Position.Left;
    };
  }

  static std::function<float(const FBandBoundingBox&, const FBandBoundingBox&)>
  GetSimilarityMeasure() {
    return [](const FBandBoundingBox& prev, const FBandBoundingBox& cur) {
      return prev.GetSimilarity(cur);
    };
  }

  static std::function<float(const FBandBoundingBox&, const FBandBoundingBox&)>
  GetDisplacementMeasure() {
    return [](const FBandBoundingBox& prev, const FBandBoundingBox& cur) {
      return prev.GetDistance(cur);
    };
  }

  static std::function<float(const FBandBoundingBox&, const FBandBoundingBox&)>
  GetLandmarkDisplacementMeasure() {
    return [](const FBandBoundingBox& prev,
              const FBandBoundingBox& cur) -> float {
      return prev.GetLandmarkDistance(cur);
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

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float Displacement;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float LandmarkDisplacement;
};

inline bool operator<(const FBandBoundingBox& Lhs,
                      const FBandBoundingBox& Rhs) {
  return Lhs.Confidence > Rhs.Confidence;
}
