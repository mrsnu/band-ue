#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "BandModelFactory.generated.h"

UCLASS()
class BANDEDITOR_API UBandModelFactory : public UFactory {
  GENERATED_BODY()

public:
  UBandModelFactory(const FObjectInitializer& ObjectInitializer);
  virtual bool FactoryCanImport(const FString& Filename) override;
  virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent,
                                       FName InName, EObjectFlags Flags,
                                       UObject* Context, const TCHAR* Type,
                                       const uint8*& Buffer,
                                       const uint8* BufferEnd,
                                       FFeedbackContext* Warn) override;
};
