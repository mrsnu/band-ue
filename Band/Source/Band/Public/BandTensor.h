#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "BandLibrary/BandLibrary.h"
#include "BandTensor.generated.h"

UCLASS(Blueprintable)
class BAND_API UBandTensor : public UObject
{
	GENERATED_BODY()

	virtual void BeginDestroy() override;

	TfLiteTensor* TensorHandle = nullptr;
	friend class FBandModule;
public:

	TfLiteType Type();
	int32 NumDims();
	uint64 ByteSize();
	void* Data();
	FString Name();

	TfLiteStatus CopyFromBuffer(const TArray<uint8>& Buffer);
	TfLiteStatus CopyToBuffer(TArray<uint8>& Buffer);
};