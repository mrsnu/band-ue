// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Modules/ModuleManager.h"
#include "BandLibrary.h"
#include "BandEnum.h"
#include "BandInterpreterComponent.h"

struct TfLiteTensor;

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
	FString GetVersion();
	Band::TfLiteInterpreter* GetInterpreterHandle() const;
	
private:
	bool InitializeInterpreter(FString ConfigPath);
	bool LoadDllFunction(FString LibraryPath);

	static void ReportError(void* UserData, const char* Format, va_list Args);
	void OnEndInvokeInternal(int32 JobId, TfLiteStatus Status) const;

	/* Reference to interpreter. Mainly for callback propagation */
	void RegisterInterpreter(UBandInterpreterComponent* Interpreter);
	void UnregisterInterpreter(UBandInterpreterComponent* Interpreter);
	TWeakObjectPtr<UBandInterpreterComponent> Interpreter = nullptr;
	
	Band::TfLiteInterpreter* InterpreterHandle = nullptr;
	void* LibraryHandle = nullptr;
	bool IsDllLoaded = false;

	friend class UBandTensor;
	friend class UBandModel;
	friend class UBandInterpreterComponent;
	/*
		DLL handles from Band Library
	*/
	Band::pTfLiteVersion TfLiteVersion = nullptr;
	Band::pTfLiteModelCreate TfLiteModelCreate = nullptr;
	Band::pTfLiteModelCreateFromFile TfLiteModelCreateFromFile = nullptr;
	Band::pTfLiteModelDelete TfLiteModelDelete = nullptr;
	Band::pTfLiteInterpreterOptionsCreate TfLiteInterpreterOptionsCreate = nullptr;
	Band::pTfLiteInterpreterOptionsDelete TfLiteInterpreterOptionsDelete = nullptr;
	Band::pTfLiteInterpreterOptionsSetOnInvokeEnd TfLiteInterpreterOptionsSetOnInvokeEnd = nullptr;
	Band::pTfLiteInterpreterOptionsSetConfigPath TfLiteInterpreterOptionsSetConfigPath = nullptr;
	Band::pTfLiteInterpreterOptionsSetConfigFile TfLiteInterpreterOptionsSetConfigFile = nullptr;
	Band::pTfLiteInterpreterOptionsSetErrorReporter TfLiteInterpreterOptionsSetErrorReporter = nullptr;
	Band::pTfLiteInterpreterCreate TfLiteInterpreterCreate = nullptr;
	Band::pTfLiteInterpreterDelete TfLiteInterpreterDelete = nullptr;
	Band::pTfLiteInterpreterRegisterModel TfLiteInterpreterRegisterModel = nullptr;
	Band::pTfLiteInterpreterInvokeSync TfLiteInterpreterInvokeSync = nullptr;
	Band::pTfLiteInterpreterInvokeAsync TfLiteInterpreterInvokeAsync = nullptr;
	Band::pTfLiteInterpreterWait TfLiteInterpreterWait = nullptr;
	Band::pTfLiteInterpreterGetInputTensorCount TfLiteInterpreterGetInputTensorCount = nullptr;
	Band::pTfLiteInterpreterGetOutputTensorCount TfLiteInterpreterGetOutputTensorCount = nullptr;
	Band::pTfLiteInterpreterAllocateInputTensor TfLiteInterpreterAllocateInputTensor = nullptr;
	Band::pTfLiteInterpreterAllocateOutputTensor TfLiteInterpreterAllocateOutputTensor = nullptr;
	Band::pTfLiteTensorDeallocate TfLiteTensorDeallocate = nullptr;
	Band::pTfLiteTensorType TfLiteTensorType = nullptr;
	Band::pTfLiteTensorNumDims TfLiteTensorNumDims = nullptr;
	Band::pTfLiteTensorDim TfLiteTensorDim = nullptr;
	Band::pTfLiteTensorByteSize TfLiteTensorByteSize = nullptr;
	Band::pTfLiteTensorData TfLiteTensorData = nullptr;
	Band::pTfLiteTensorName TfLiteTensorName = nullptr;
	Band::pTfLiteTensorQuantizationParams TfLiteTensorQuantizationParams = nullptr;
	Band::pTfLiteTensorCopyFromBuffer TfLiteTensorCopyFromBuffer = nullptr;
	Band::pTfLiteTensorCopyToBuffer TfLiteTensorCopyToBuffer = nullptr;
};
