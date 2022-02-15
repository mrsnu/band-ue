#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

namespace Band {
	struct TfLiteModel;
}

UCLASS()
class BAND_API UBandModel : public UObject
{
	GENERATED_BODY()

	Band::TfLiteModel* ModelHandle = nullptr;
public:
	static UBandModel* LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size);
};