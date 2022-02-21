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
	static int32 GetInputTensorCount(UPARAM(ref) UBandModel *Model);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 GetOutputTensorCount(UPARAM(ref) UBandModel *Model);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static UBandTensor *AllocateInputTensor(UPARAM(ref) UBandModel *Model, int32 InputIndex);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static UBandTensor *AllocateOutputTensor(UPARAM(ref) UBandModel *Model, int32 OutputIndex);

	// TODO(dostos): How to save output tensors?
	UFUNCTION(BlueprintCallable, Category = "Band")
	static void InvokeSync(UPARAM(ref) UBandModel *Model, UPARAM(ref) TArray<UBandTensor *> InputTensors, UPARAM(ref) TArray<UBandTensor *> OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 InvokeAsync(UPARAM(ref) UBandModel *Model, UPARAM(ref) TArray<UBandTensor *> InputTensors, UPARAM(ref) TArray<UBandTensor *> OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor *> OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandTensorType GetTensorType(UBandTensor *Tensor);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 NumDims(UBandTensor *Tensor);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static int32 ByteSize(UBandTensor *Tensor);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static TArray<uint8> Data(UBandTensor *Tensor);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static FString Name(UBandTensor *Tensor);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus CopyFromBuffer(UBandTensor *Tensor, TArray<uint8> Buffer);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus CopyToBuffer(UBandTensor *Tensor, TArray<uint8> Buffer);
};