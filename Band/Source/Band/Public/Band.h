// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Modules/ModuleManager.h"
#include "BandLibrary.h"
#include "BandEnum.h"
#include "BandInterpreter.h"

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

	UBandInterpreter* GetInterpreter();

	/* Inpterpreter interfaces */
	FString GetVersion();
	int32 GetInputTensorCount(UBandModel* Model);
	int32 GetOutputTensorCount(UBandModel* Model);
	UBandTensor* AllocateInputTensor(UBandModel* Model, int32 InputIndex);
	UBandTensor* AllocateOutputTensor(UBandModel* Model, int32 OutputIndex);

	void InvokeSync(UBandModel* Model, TArray<UBandTensor*> InputTensors, TArray<UBandTensor*> OutputTensors);
	int32 InvokeAsync(UBandModel* Model, TArray<UBandTensor*> InputTensors);
	EBandStatus Wait(int32 JobHandle, TArray<UBandTensor*> OutputTensors);

	// Registers model using given asset
	// Returns model handle (-1 : invalid)
	int32 RegisterModel(UBandModel* Model) const;

private:
	bool InitializeInterpreter(FString ConfigPath);
	bool LoadDllFunction(FString LibraryPath);
	// Callback function for TfLiteErrorReporter
	static void ReportError(void* UserData, const char* Format, va_list Args);
	void OnEndInvokeInternal(int JobId, TfLiteStatus Status) const;
	
	TArray<TfLiteTensor*> TensorsFromTArray(TArray<UBandTensor*> Tensors);

	UBandInterpreter* Interpreter = nullptr;
	void* LibraryHandle = nullptr;
	bool IsDllLoaded = false;
};
