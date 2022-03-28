#pragma once

#include "Band.h"
#include "BandTensor.h"
#include "BandEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Band")
	static FString GetVersion();

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor *> OutputTensors);
};