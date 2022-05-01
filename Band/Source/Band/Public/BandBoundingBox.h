// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"
#include "Rect.h"
#include "BandBoundingBox.generated.h"

UCLASS(Blueprintable)
class BAND_API UBandBoundingBox : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Label;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Confidence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRect Position;

	void InitBandBoundingBox(float Cf, float Left, float Bottom, float Right, float Top);

	UFUNCTION(BlueprintCallable)
	void PrintBox();

	UFUNCTION(BlueprintCallable)
	void ParseRectF(const int ImageHeight, const int ImageWidth, float& PosX, float& PosY, float& SizeX, float& SizeY);
};