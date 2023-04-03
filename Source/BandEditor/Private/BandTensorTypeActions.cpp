#include "BandTensorTypeActions.h"
#include "BandTensor.h"

FBandTensorTypeActions::FBandTensorTypeActions(
    EAssetTypeCategories::Type InAssetCategory)
  : BandAssetCategory(InAssetCategory) {
}

FText FBandTensorTypeActions::GetName() const {
  return FText::FromString("Band TfLite Tensor");
}

FColor FBandTensorTypeActions::GetTypeColor() const {
  return FColor(200, 165, 205);
}

UClass* FBandTensorTypeActions::GetSupportedClass() const {
  return UBandTensor::StaticClass();
}

uint32 FBandTensorTypeActions::GetCategories() {
  return BandAssetCategory;
}
