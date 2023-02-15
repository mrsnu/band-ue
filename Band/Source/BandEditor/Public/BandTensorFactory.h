#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "BandTensorFactory.generated.h"

UCLASS()
class BANDEDITOR_API UBandTensorFactory : public UFactory {
  GENERATED_BODY()

public:
  UBandTensorFactory(const FObjectInitializer& ObjectInitializer);
};
