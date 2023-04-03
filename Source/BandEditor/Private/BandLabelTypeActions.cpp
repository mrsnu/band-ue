#include "BandLabelTypeActions.h"
#include "BandLabel.h"

FBandLabelTypeActions::FBandLabelTypeActions(
    EAssetTypeCategories::Type InAssetCategory)
  : BandAssetCategory(InAssetCategory) {
}

FText FBandLabelTypeActions::GetName() const {
  return FText::FromString("Band TfLite Label");
}

FColor FBandLabelTypeActions::GetTypeColor() const {
  return FColor(230, 205, 165);
}

UClass* FBandLabelTypeActions::GetSupportedClass() const {
  return UBandModel::StaticClass();
}

uint32 FBandLabelTypeActions::GetCategories() {
  return BandAssetCategory;
}
