// Fill out your copyright notice in the Description page of Project Settings.

#include "BandBoundingBox.h"
#include <cmath>

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
  FJsonObjectConverter::UStructToJsonObjectString(*this, json_string);
  return json_string;
}

float FBandBoundingBox::GetWidth() const {
  return std::abs(Position.Right - Position.Left);
}

float FBandBoundingBox::GetHeight() const {
  return std::abs(Position.Top - Position.Bottom);
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
