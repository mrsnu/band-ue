#pragma once

#include "Modules/ModuleManager.h"
#include "UObject/ObjectMacros.h"
#include "BandLabel.generated.h"


UCLASS(Blueprintable)
class BAND_API UBandLabel : public UDataAsset {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  TArray<FString> Labels;

public:
  UFUNCTION(BlueprintCallable, Category = Band)
  FString GetClassName(int32 Index) const;

  UFUNCTION(BlueprintCallable, Category = Band)
  int32 GetNumClasses() const { return Labels.Num(); }

  static UBandLabel *LoadLabel(UObject *InParent, FName InName,
                               EObjectFlags Flags, const FString &Filename,
                               bool &bOutOperationCanceled);
};
