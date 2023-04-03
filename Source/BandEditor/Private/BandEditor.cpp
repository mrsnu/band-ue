// Copyright Epic Games, Inc. All Rights Reserved.

#include "BandEditor.h"
#include "Band.h"
#include "BandModel.h"
#include "BandModelTypeActions.h"
#include "BandTensorTypeActions.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"

#include <iostream>

#define LOCTEXT_NAMESPACE "FBandEditorModule"

void FBandEditorModule::StartupModule() {
  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<
    FAssetToolsModule>("AssetTools").Get();
  EAssetTypeCategories::Type BandCategory = AssetTools.
      RegisterAdvancedAssetCategory(FName(TEXT("Band")),
                                    FText::FromString("Band"));
  // register our custom asset with example category
  TSharedPtr<IAssetTypeActions> ModelAction = MakeShareable(
      new FBandModelTypeActions(BandCategory));
  AssetTools.RegisterAssetTypeActions(ModelAction.ToSharedRef());
  TSharedPtr<IAssetTypeActions> TensorAction = MakeShareable(
      new FBandTensorTypeActions(BandCategory));
  AssetTools.RegisterAssetTypeActions(TensorAction.ToSharedRef());
  // saved it here for unregister later
  CreatedAssetTypeActions.Add(ModelAction);
  CreatedAssetTypeActions.Add(TensorAction);
}

void FBandEditorModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.

  // Unregister all the asset types that we registered
  if (FModuleManager::Get().IsModuleLoaded("AssetTools")) {
    IAssetTools& AssetTools = FModuleManager::GetModuleChecked<
      FAssetToolsModule>("AssetTools").Get();
    for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i) {
      AssetTools.UnregisterAssetTypeActions(
          CreatedAssetTypeActions[i].ToSharedRef());
    }
  }
  CreatedAssetTypeActions.Empty();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBandEditorModule, BandEditor)
