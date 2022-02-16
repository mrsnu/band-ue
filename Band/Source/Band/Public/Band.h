// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"

namespace Band {
	struct TfLiteInterpreter;
}

class BAND_API FBandModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	bool LoadDllFunction(FString LibraryPath);
	// Callback function for TfLiteErrorReporter
	static void ReportError(void* user_data, const char* format, va_list args);

	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
	Band::TfLiteInterpreter* Interpreter = nullptr;
	void* LibraryHandle = nullptr;
	bool IsDllLoaded = false;
};
