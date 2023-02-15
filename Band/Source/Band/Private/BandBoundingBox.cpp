// Fill out your copyright notice in the Description page of Project Settings.

#include "BandBoundingBox.h"
#include "GenericPlatform/GenericPlatformMath.h"

FBandBoundingBox::FBandBoundingBox(float Confidence, FRect Position,
                                   FString Label,
                                   TArray<FBandLandmark> Landmark)
  : Confidence(Confidence),
    Position(Position),
    Label(Label),
    Landmark(Landmark) {
}
