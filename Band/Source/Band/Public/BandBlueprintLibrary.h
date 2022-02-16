#pragma once

#include "Band.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="Band")
	static FString GetVersion();

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 GetInputTensorCount(UBandModel* Model);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 GetOutputTensorCount(UBandModel* Model);
};