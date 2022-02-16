#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

namespace Band {
	struct TfLiteModel;
}

UCLASS(Blueprintable)
class BAND_API UBandModel : public UObject
{
	GENERATED_BODY()

	int32 ModelHandle = -1;
	
public:
	const int32 GetModelHandle() const;
	static UBandModel* LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size);
};