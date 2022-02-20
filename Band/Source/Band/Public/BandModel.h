#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

namespace Band {
	struct TfLiteModel;
}

// TODO(dostos): Make sure variable of this class is non-nullable in BP.
UCLASS(Blueprintable)
class BAND_API UBandModel : public UDataAsset
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 ModelHandle = -1;

	UPROPERTY(Transient)
	bool Registered = false;

	UPROPERTY()
	TArray<uint8> ModelBinary;
	
public:
	bool IsRegistered() const;
	const int32 GetModelHandle();
	void RegisterModel();

	static UBandModel* LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size);
};