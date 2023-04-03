#pragma once

#include <mutex>

#include "UObject/ObjectMacros.h"
#include "Modules/ModuleManager.h"
#include "BandModel.generated.h"

class UBandTensor;
struct BandModel;

// TODO(dostos): Make sure variable of this class is non-nullable in BP.
UCLASS(Blueprintable)
class BAND_API UBandModel : public UDataAsset {
  GENERATED_BODY()

public:
  virtual void BeginDestroy() override;

  BandModel* GetHandle() const;
  const TArray<uint8>& GetBinary() const;

  UFUNCTION(BlueprintCallable, Category = "Band")
  bool IsRegistered() const;
  UFUNCTION(BlueprintCallable, Category = "Band")
  void RegisterModel() const;

  static UBandModel* LoadModel(UObject* InParent, FName InName,
                               EObjectFlags Flags, const uint8*& Buffer,
                               size_t Size);

private:
  mutable BandModel* ModelHandle = nullptr;
  mutable std::mutex RegisterMutex;

  UPROPERTY()
  TArray<uint8> ModelBinary;
};
