// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"

class BANDEDITOR_API FBandEditorModule : public IModuleInterface {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
};
