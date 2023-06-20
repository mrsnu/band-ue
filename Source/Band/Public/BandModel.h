#pragma once

#include <mutex>

#include "BandModel.generated.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectMacros.h"


class UBandTensor;
struct BandModel;

// TODO(dostos): Make sure variable of this class is non-nullable in BP.
UCLASS(Blueprintable)
class BAND_API UBandModel : public UDataAsset {
  GENERATED_BODY()

public:
  virtual void BeginDestroy() override;

  UFUNCTION(BlueprintCallable, Category = "Band")
  int32 GetInputTensorCount() const;
  UFUNCTION(BlueprintCallable, Category = "Band")
  int32 GetOutputTensorCount() const;

  UFUNCTION(BlueprintCallable, Category = "Band")
  UBandTensor *AllocateInputTensor(int32 InputIndex) const;
  UFUNCTION(BlueprintCallable, Category = "Band")
  UBandTensor *AllocateOutputTensor(int32 OutputIndex) const;

  UFUNCTION(BlueprintCallable, Category = "Band")
  TArray<UBandTensor *> AllocateInputTensors() const;
  UFUNCTION(BlueprintCallable, Category = "Band")
  TArray<UBandTensor *> AllocateOutputTensors() const;

private:
  friend class FBandModule;
  friend class UBandModelFactory;

  BandModel *GetHandle() const;
  static UBandModel *LoadModel(UObject *InParent, FName InName,
                               EObjectFlags Flags, const uint8 *&Buffer,
                               size_t Size);

  const TArray<uint8> &GetBinary() const;

  mutable std::mutex RegisterMutex;
  mutable BandModel *Handle = nullptr;

  UPROPERTY()
  TArray<uint8> ModelBinary;
};
