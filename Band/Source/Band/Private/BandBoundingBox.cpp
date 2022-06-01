// Fill out your copyright notice in the Description page of Project Settings.

#include "BandBoundingBox.h"
#include "GenericPlatform/GenericPlatformMath.h"

void FBandBoundingBox::InitBandBoundingBox(float Cf, float Left, float Bottom, float Right, float Top)
{
	Confidence = Cf;
	Position.Left = Left;
	Position.Bottom = Bottom;
	Position.Right = Right;
	Position.Top = Top;
}
