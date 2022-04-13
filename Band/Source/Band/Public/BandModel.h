#pragma once

#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

class UBandTensor;

namespace Band {
	struct TfLiteModel;
}

// TODO(dostos): Make sure variable of this class is non-nullable in BP.
UCLASS(Blueprintable)
class BAND_API UBandModel : public UDataAsset
{
	GENERATED_BODY()
	
public:
	bool IsRegistered() const;
	const int32 GetModelHandle();
	void RegisterModel();

	UFUNCTION(BlueprintCallable, Category = "Band")
	UBandTensor* AllocateInputTensor(int32 InputIndex);

	UFUNCTION(BlueprintCallable, Category = "Band")
	UBandTensor* AllocateOutputTensor(int32 OutputIndex);

	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 GetInputTensorCount();

	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 GetOutputTensorCount();

	UFUNCTION(BlueprintCallable, Category = "Band")
	void InvokeSync(UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 InvokeAsync(UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors);

	static UBandModel* LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size);

private:
	UPROPERTY(VisibleAnywhere)
	int32 ModelHandle = -1;

	UPROPERTY(Transient)
	bool Registered = false;

	UPROPERTY()
	TArray<uint8> ModelBinary;
};