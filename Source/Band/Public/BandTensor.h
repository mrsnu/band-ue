#pragma once

#include <memory>

#include "AndroidCameraFrame.h"
#include "BandBoundingBox.h"
#include "BandEnum.h"
#include "BandLibrary.h"
#include "UObject/ObjectMacros.h"
#include "BandTensor.generated.h"

namespace band {
class FrameBuffer;
}

UCLASS(Blueprintable)
class BAND_API UBandTensor : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = Band)
  void FromCameraFrame(UPARAM(ref) const UAndroidCameraFrame* Frame,
                       const float Mean = 0.0f,
                       const float Std = 1.0f);
  UFUNCTION(BlueprintCallable, Category = Band)
  void FromCameraFrameWithCrop(UPARAM(ref) const UAndroidCameraFrame* Frame,
                               FBandBoundingBox RoI,
                               const float Mean = 0.0f,
                               const float Std = 1.0f);

  UFUNCTION(BlueprintCallable, Category = Band)
  void FromBoundingBox(UPARAM(ref) const FBandBoundingBox Box);

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
  TArray<T> GetBuffer() {
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
  EBandStatus CopyFromTexture(UPARAM(ref) UTexture2D* Texture, float Mean,
                              float Std = 1.f);
  UFUNCTION(BlueprintCallable, Category = "Band")
  EBandStatus CopyFromTextureWithCrop(UPARAM(ref) UTexture2D* Texture,
                                      FBandBoundingBox BBox,
                                      float Mean = 127.5f,
                                      float Std = 127.5f);
  UFUNCTION(BlueprintCallable, Category = "Band")
  EBandStatus CopyToBuffer(TArray<uint8> Buffer);

  BandTensor* Handle() const;

private:
  friend class UBandInterfaceComponent;

  void Initialize(BandTensor* TensorHandle);
  EBandStatus CopyFromFrameBuffer(
      std::unique_ptr<band::FrameBuffer> SourceFrameBuffer,
      FBandBoundingBox RoI, float Mean, float Std);
  virtual void BeginDestroy() override;

  BandTensor* TensorHandle = nullptr;

  uint8* RGBBuffer = nullptr;
};
