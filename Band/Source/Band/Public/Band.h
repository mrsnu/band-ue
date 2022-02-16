// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

namespace Band {
	struct TfLiteInterpreter;
	struct TfLiteModel;
}
class UBandModel;

class BAND_API FBandModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/* Returns singleton object (Note: avoid calling this in shutdown phase) */
	static FBandModule& Get();

	FString GetVersion();
	int32 GetInputTensorCount(UBandModel* Model);
	int32 GetOutputTensorCount(UBandModel* Model);
	int32 RegisterModel(Band::TfLiteModel* ModelPtr);

private:
	bool LoadDllFunction(FString LibraryPath);
	// Callback function for TfLiteErrorReporter
	static void ReportError(void* user_data, const char* format, va_list args);

	Band::TfLiteInterpreter* Interpreter = nullptr;
	void* LibraryHandle = nullptr;
	bool IsDllLoaded = false;
};
