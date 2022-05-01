// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Modules/ModuleManager.h"
#include "BandLibrary.h"
#include "BandEnum.h"

namespace Band
{
	struct TfLiteInterpreter;
}
struct TfLiteTensor;
class UBandModel;
class UBandTensor;

DECLARE_LOG_CATEGORY_EXTERN(LogBand, Log, All);
DECLARE_STATS_GROUP(TEXT("Band"), STATGROUP_Band, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("CameraToTensor"), STAT_BandCameraToTensor, STATGROUP_Band, BAND_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("TextureToTensor"), STAT_BandTextureToTensor, STATGROUP_Band, BAND_API);

/*
	DLL interfaces for Band / owns main interpreter
*/
class BAND_API FBandModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/* Returns singleton object (Note: avoid calling this in shutdown phase) */
	static FBandModule& Get();

	bool InitializeInterpreter(FString ConfigPath);

	/* Inpterpreter interfaces */
	FString GetVersion();
	int32 GetInputTensorCount(UBandModel* Model);
	int32 GetOutputTensorCount(UBandModel* Model);
	UBandTensor* AllocateInputTensor(UBandModel* Model, int32 InputIndex);
	UBandTensor* AllocateOutputTensor(UBandModel* Model, int32 OutputIndex);

	void InvokeSync(UBandModel* Model, TArray<UBandTensor*> InputTensors, TArray<UBandTensor*> OutputTensors);
	int32 InvokeAsync(UBandModel* Model, TArray<UBandTensor*> InputTensors);
	EBandStatus Wait(int32 JobHandle, TArray<UBandTensor*> OutputTensors);

	/* Helper functions for UBandModel */
	Band::TfLiteInterpreter* GetInterpreter();

private:
	TArray<TfLiteTensor*> TensorsFromTArray(TArray<UBandTensor*> Tensors);
	bool LoadDllFunction(FString LibraryPath);
	// Callback function for TfLiteErrorReporter
	static void ReportError(void* UserData, const char* Format, va_list Args);
	static void ReportError(void *user_data, const char *format, va_list args);

	Band::TfLiteInterpreter* Interpreter = nullptr;
	void* LibraryHandle = nullptr;
	bool IsDllLoaded = false;
};
