#pragma once

#include "UObject/ObjectMacros.h"
#include "BandLibrary/BandLibrary.h"
#include "BandEnum.h"
#include "BandTensor.generated.h"

UCLASS(Blueprintable)
class BAND_API UBandTensor : public UObject
{
	GENERATED_BODY()

	virtual void BeginDestroy() override;

	TfLiteTensor* TensorHandle = nullptr;
	friend class FBandModule;
public:

	EBandTensorType Type();
	int32 NumDims();
	uint64 ByteSize();
	TArray<uint8> Data();
	FString Name();

	EBandStatus CopyFromBuffer(TArray<uint8> Buffer);
	EBandStatus CopyToBuffer(TArray<uint8> Buffer);
};