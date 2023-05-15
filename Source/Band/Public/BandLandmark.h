// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "BandEnum.h"
#include "BandLandmark.generated.h"

// Based on Mediapipe's landmark.proto
USTRUCT(BlueprintType)
struct BAND_API FBandLandmark {
  GENERATED_BODY()

  FBandLandmark() = default;
  FBandLandmark(float X, float Y, float Z, float Confidence);

  float GetDistance(const FBandLandmark& Rhs) const;
  bool operator==(const FBandLandmark& rhs) const;

  static std::vector<size_t> GetImportantLandmarks(
      EBandLandmark Type);

  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  float Confidence = 0.0;
  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  FVector Point; // Merges X, Y, Z into a single FVector
};
