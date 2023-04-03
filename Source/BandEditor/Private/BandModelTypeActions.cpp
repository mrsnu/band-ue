#include "BandModelTypeActions.h"
#include "BandModel.h"

FBandModelTypeActions::FBandModelTypeActions(
    EAssetTypeCategories::Type InAssetCategory)
  : BandAssetCategory(InAssetCategory) {
}

FText FBandModelTypeActions::GetName() const {
  return FText::FromString("Band TfLite Model");
}

FColor FBandModelTypeActions::GetTypeColor() const {
  return FColor(230, 205, 165);
}

UClass* FBandModelTypeActions::GetSupportedClass() const {
  return UBandModel::StaticClass();
}

uint32 FBandModelTypeActions::GetCategories() {
  return BandAssetCategory;
}
