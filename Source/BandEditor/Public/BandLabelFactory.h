#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "BandLabelFactory.generated.h"

UCLASS()
class BANDEDITOR_API UBandLabelFactory : public UFactory {
  GENERATED_BODY()

public:
  UBandLabelFactory(const FObjectInitializer& ObjectInitializer);
  virtual bool FactoryCanImport(const FString& Filename) override;
  virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent,
                                     FName InName, EObjectFlags Flags,
                                     const FString& Filename,
                                     const TCHAR* Parms, FFeedbackContext* Warn,
                                     bool& bOutOperationCanceled) override;
};
