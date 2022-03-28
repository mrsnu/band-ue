// Fill out your copyright notice in the Description page of Project Settings.


#include "BandBoundingBox.h"
#include "GenericPlatform/GenericPlatformMath.h"

void UBandBoundingBox::InitBandBoundingBox(float Cf, float Left, float Bottom, float Right, float Top) {
	Confidence = Cf;
	Position.Left = Left;
	Position.Bottom = Bottom;
	Position.Right = Right;
	Position.Top = Top;
}

void UBandBoundingBox::PrintBox() {
	UE_LOG(LogBand, Log, TEXT("UBandBoundingBox: %f - (%f, %f, %f, %f)"), Confidence,
		Position.Left, Position.Right, Position.Top, Position.Bottom);
}

void UBandBoundingBox::ParseRectF(const int ImageHeight, const int ImageWidth, float &PosX, float &PosY, float &SizeX, float &SizeY) {
	float Left = Position.Left;
	float Right = Position.Right;
	float Top = Position.Top;
	float Bottom = Position.Bottom;

	SizeX = (Right - Left) * ImageWidth;
	SizeY = (Bottom - Top) * ImageHeight;

	PosX = Left * ImageWidth;
	PosY = Top * ImageHeight;

	if (SizeX < 0) { // Shift PosX to the left by SizeX
		PosX += SizeX;
	}
	if (SizeY < 0) { // Shift PoxY upwards by SizeY
		PosY += SizeY;
	}

	SizeX = FGenericPlatformMath::Abs(SizeX);
	SizeY = FGenericPlatformMath::Abs(SizeY);

	UE_LOG(LogBand, Log, TEXT("UBandBoundingBox: ParseRectF (%f, %f, %f, %f)"), PosX, PosY, SizeX, SizeY);
}
