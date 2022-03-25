#pragma once

#include "UObject/ObjectMacros.h"
#include "BandLibrary.h"
#include "BandEnum.h"

#include "AndroidCameraFrame.h"
#include "BandTensor.generated.h"

UCLASS(Blueprintable)
class BAND_API UBandTensor : public UObject
{
	GENERATED_BODY()

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = Band)
	void FromCameraFrame(const UAndroidCameraFrame* Frame, bool Normalize = false);

	UFUNCTION(BlueprintCallable, Category = Band)
	void ArgMax(int32& Index, float& Value);

	void Initialize(TfLiteTensor* TensorHandle);
	TfLiteTensor* TensorHandle = nullptr;
	friend class FBandModule;

	uint8* RGBBuffer = nullptr;

public:
	EBandTensorType Type();
	int32 Dim(int32 Index);
	int32 NumDims();
	int32 NumElements();
	int32 ByteSize();
	uint8* Data();
	FString Name();

	EBandStatus CopyFromBuffer(uint8* Buffer, int32 Bytes);
	EBandStatus CopyFromBuffer(TArray<uint8> Buffer);
	EBandStatus CopyFromTexture(UTexture2D* Texture, float Mean, float Std);
	EBandStatus CopyToBuffer(TArray<uint8> Buffer);
};