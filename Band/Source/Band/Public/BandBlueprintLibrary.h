#pragma once

#include "Band.h"
#include "BandTensor.h"
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

	UFUNCTION(BlueprintCallable, Category = "Band")
	static UBandTensor* AllocateInputTensor(UBandModel* Model, int32 InputIndex);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static UBandTensor* AllocateOutputTensor(UBandModel* Model, int32 OutputIndex);

	// TODO(dostos): How to save output tensors?
	UFUNCTION(BlueprintCallable, Category = "Band")
	static void InvokeSync(UBandModel* Model, const TArray<UBandTensor*>& InputTensors, TArray<UBandTensor*>& OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 InvokeAsync(UBandModel* Model, const TArray<UBandTensor*>& InputTensors, TArray<UBandTensor*>& OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus Wait(int32 JobHandle, TArray<UBandTensor*>& OutputTensors);
};