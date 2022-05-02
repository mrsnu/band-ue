#pragma once

#include <mutex>

#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

class UBandTensor;

namespace Band
{
	struct TfLiteModel;
}

// TODO(dostos): Make sure variable of this class is non-nullable in BP.
UCLASS(Blueprintable)
class BAND_API UBandModel : public UDataAsset
{
	GENERATED_BODY()

public:
	const int32 GetHandle();
	const TArray<uint8>& GetBinary() const;
	
	UFUNCTION(BlueprintCallable, Category = "Band")
	bool IsRegistered() const;
	UFUNCTION(BlueprintCallable, Category = "Band")
	void RegisterModel();

	static UBandModel* LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size);

private:
	UPROPERTY(VisibleAnywhere)
	int32 ModelHandle = -1;

	std::mutex RegisterMutex;
	// TODO(dostos): replace this with (Handle != -1)
	UPROPERTY(Transient)
	bool Registered = false;

	UPROPERTY()
	TArray<uint8> ModelBinary;
};