// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

#include "BandLandmark.h"
#include "Rect.h"
#include "UObject/ObjectMacros.h"
#include "BandBoundingBox.generated.h"


USTRUCT(BlueprintType)
struct BAND_API FBandBoundingBox {
  GENERATED_BODY()

  FBandBoundingBox() = default;
  FBandBoundingBox(float Confidence, FRect Position, FString Label = TEXT(""),
                   // Default to have low priority
                   TArray<FBandLandmark> Landmark = {},
                   TArray<FIntPoint> LandmarkEdge = {});

  bool operator==(const FBandBoundingBox &Rhs) const;
  FString ToString() const;

  FVector GetSize() const;
  FVector GetCenter() const;
  // [Left, Top, 0]
  FVector GetOrigin() const;
  // [Right, Bottom, 0]
  FVector GetEnd() const;

  float GetDiagonal() const;
  float GetWidth() const;
  float GetHeight() const;
  float GetCenterX() const;
  float GetCenterY() const;
  float GetSimilarity(const FBandBoundingBox &Rhs) const;
  float GetDistance(const FBandBoundingBox &Rhs) const;
  float GetLandmarkDistance(const FBandBoundingBox &Rhs) const;
  void GetLandmarkFrom(const FBandBoundingBox &Rhs);

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

inline bool operator<(const FBandBoundingBox &Lhs,
                      const FBandBoundingBox &Rhs) {
  return Lhs.Confidence > Rhs.Confidence;
}
