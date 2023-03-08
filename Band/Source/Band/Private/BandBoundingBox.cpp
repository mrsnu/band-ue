// Fill out your copyright notice in the Description page of Project Settings.

#include "BandBoundingBox.h"
#include <cmath>
#include "Rect.h"

FBandBoundingBox::FBandBoundingBox(float Confidence, FRect Position,
                                   FString Label,
                                   TArray<FBandLandmark> Landmark,
                                   TArray<FIntPoint> LandmarkEdge)
  : Confidence(Confidence),
    Position(Position),
    Label(Label),
    Landmark(Landmark),
    LandmarkEdge(LandmarkEdge) {
}

bool FBandBoundingBox::operator==(const FBandBoundingBox& Rhs) const {
  return (Confidence == Rhs.Confidence) && (Position == Rhs.Position) && (
           Label == Rhs.Label) && (Landmark == Rhs.Landmark) && (
           LandmarkEdge == Rhs.LandmarkEdge);
}

FString FBandBoundingBox::ToString() const {
  return FString::Printf(
      TEXT(
          "FBandBoundingBox [%f %f %f %f] with confidence %f"),
      Position.Bottom, Position.Top,
      Position.Left,
      Position.Right, Confidence);
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
  // otherwise return center_distance
  const float center_distance = std::sqrt(
      std::pow(GetCenterX() - Rhs.GetCenterX(), 2) +
      std::pow(GetCenterY() - Rhs.GetCenterY(), 2));
  return center_distance;
}
