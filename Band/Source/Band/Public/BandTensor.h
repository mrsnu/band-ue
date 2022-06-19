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
public:
	UFUNCTION(BlueprintCallable, Category = Band)
	void FromCameraFrame(UPARAM(ref) const UAndroidCameraFrame* Frame, bool Normalize = false);

	UFUNCTION(BlueprintCallable, Category = Band)
	void ArgMax(int32& Index, float& Value);

	UFUNCTION(BlueprintCallable, Category = "Band")
	EBandTensorType Type();
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 Dim(int32 Index);
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 NumDims();
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 NumElements();
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 ByteSize();
	UFUNCTION(BlueprintCallable, Category = "Band")
	FString Name();
	uint8* Data();

	template <typename T>
	TArray<T> GetBuffer()
	{
		return TArray<T>(reinterpret_cast<T*>(Data()), ByteSize() / sizeof(T));
	}
	
	UFUNCTION(BlueprintCallable, Category = "Band")
	TArray<uint8> GetUInt8Buffer();
	UFUNCTION(BlueprintCallable, Category = "Band")
	TArray<float> GetF32Buffer();

	EBandStatus CopyFromBuffer(uint8* Buffer, int32 Bytes);
	UFUNCTION(BlueprintCallable, Category = "Band")
	EBandStatus CopyFromBuffer(TArray<uint8> Buffer);
	UFUNCTION(BlueprintCallable, Category = "Band")
	EBandStatus CopyFromTexture(UPARAM(ref) UTexture2D* Texture, float Mean, float Std = 1.f);
	UFUNCTION(BlueprintCallable, Category = "Band")
	EBandStatus CopyToBuffer(TArray<uint8> Buffer);

	TfLiteTensor* Handle() const;

private:
	friend class UBandInterpreterComponent;

	void Initialize(TfLiteTensor* TensorHandle);
	virtual void BeginDestroy() override;

	TfLiteTensor* TensorHandle = nullptr;

	uint8* RGBBuffer = nullptr;
};