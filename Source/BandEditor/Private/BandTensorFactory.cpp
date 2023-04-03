#include "BandTensorFactory.h"
#include "BandTensor.h"
#include "../Public/BandTensorFactory.h"

UBandTensorFactory::UBandTensorFactory(
    const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer) {
  bText = false;
  bCreateNew = false;
  bEditAfterNew = false;
  bEditorImport = false;

  SupportedClass = UBandTensor::StaticClass();
}
