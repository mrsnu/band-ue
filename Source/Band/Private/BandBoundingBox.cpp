// Fill out your copyright notice in the Description page of Project Settings.

#include "BandBoundingBox.h"
#include <cmath>
#include <string>
#include <chrono>
#include "JsonObjectConverter.h"
#include "Rect.h"

FBandBoundingBox::FBandBoundingBox(float Confidence, FRect Position,
                                   FString Label,
                                   float Displacement,
                                   TArray<FBandLandmark> Landmark,
                                   TArray<FIntPoint> LandmarkEdge)
  : Confidence(Confidence),
    Position(Position),
    Label(Label),
    Displacement(Displacement),
    Landmark(Landmark),
    LandmarkEdge(LandmarkEdge) {
}

bool FBandBoundingBox::operator==(const FBandBoundingBox& Rhs) const {
  return (Confidence == Rhs.Confidence) && (Position == Rhs.Position) && (
           Label == Rhs.Label) && (Landmark == Rhs.Landmark) && (
           LandmarkEdge == Rhs.LandmarkEdge);
}

FString FBandBoundingBox::ToString() const {
  FString json_string;
  json_string += TEXT("{");

  json_string += FString::Printf(
      TEXT("\"confidence\": %f"), Confidence);

  json_string += TEXT(", ");
  json_string += FString::Printf(
      TEXT("\"position\": [%f, %f, %f, %f]"), Position.Top, Position.Left,
      Position.Bottom, Position.Right
      );

  json_string += TEXT(", ");
  json_string += FString::Printf(
      TEXT("\"label\": \"%s\""), *Label);

  json_string += TEXT(", ");
  json_string += TEXT("\"landmark\": [");
  for (int i = 0; i < Landmark.Num(); i++) {
    json_string += TEXT("{");
    json_string += FString::Printf(
        TEXT("\"confidence\": %f,"), Landmark[i].Confidence);
    json_string += TEXT("\"Point\": ");
    json_string += FString::Printf(
        TEXT("[%f, %f]"), Landmark[i].Point.X,
        Landmark[i].Point.Y //, Landmark[i].Point.Z
        );
    json_string += TEXT("}");
    if (i != Landmark.Num() - 1) {
      json_string += TEXT(", ");
    }
  }
  json_string += TEXT("]");

  json_string += TEXT(", ");
  json_string += TEXT("\"processing_end_ts\": ");
  json_string += std::to_string(
      std::chrono::system_clock::now().time_since_epoch().count()).c_str();

  json_string += TEXT("}");
  return json_string;
}

FVector FBandBoundingBox::GetSize() const {
  return {GetWidth(), GetHeight(), 0};
}

FVector FBandBoundingBox::GetCenter() const {
  return {GetCenterX(), GetCenterY(), 0};
}

FVector FBandBoundingBox::GetOrigin() const {
  return {Position.Left, Position.Top, 0};
}

float FBandBoundingBox::GetDiagonal() const {
  return std::sqrt(std::pow(GetWidth(), 2) + std::pow(GetHeight(), 2));
}

float FBandBoundingBox::GetWidth() const {
  return std::abs(Position.Right - Position.Left);
}

float FBandBoundingBox::GetHeight() const {
  return std::abs(Position.Bottom - Position.Top);
}

float FBandBoundingBox::GetCenterX() const {
  return (Position.Right + Position.Left) / 2;
}

float FBandBoundingBox::GetCenterY() const {
  return (Position.Bottom + Position.Top) / 2;
}

float FBandBoundingBox::GetSimilarity(const FBandBoundingBox& Rhs) const {
  const float iou = BoxIou(Position, Rhs.Position);
  // return iou if overlap
  if (iou > 0.f) {
    return -iou;
  }
  // otherwise return distance
  return GetDistance(Rhs);
}

float FBandBoundingBox::GetDistance(const FBandBoundingBox& Rhs) const {
  return std::sqrt(std::pow(GetCenterX() - Rhs.GetCenterX(), 2) +
                   std::pow(GetCenterY() - Rhs.GetCenterY(), 2));
}

float FBandBoundingBox::GetLandmarkDistance(const FBandBoundingBox& Rhs) const {
  float result = 0.f;
  check(Landmark.Num() == Rhs.Landmark.Num());
  for (size_t i = 0; i < Landmark.Num(); i++) {
    result += Landmark[i].GetDistance(Rhs.Landmark[i]);
  }
  return result / static_cast<float>(Landmark.Num());
}
