// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"
#include "Rect.h"
#include "BandLandmark.generated.h"

// Based on Mediapipe's landmark.proto
USTRUCT(BlueprintType)
struct BAND_API FBandLandmark {
  GENERATED_BODY()

  FBandLandmark() = default;

  FBandLandmark(float X, float Y, float Z, float Confidence)
    : Confidence(Confidence),
      Point(X, Y, Z) {
  };

  bool operator==(const FBandLandmark& rhs) const {
    return (Confidence == rhs.Confidence) && (Point == rhs.Point);
  }

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float Confidence = 0.0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FVector Point; // Merges X, Y, Z into a single FVector
};
