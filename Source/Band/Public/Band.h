// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Modules/ModuleManager.h"
#include "BandLibrary.h"
#include "BandEnum.h"
#include "BandInterfaceComponent.h"

struct BandTensor;

DECLARE_LOG_CATEGORY_EXTERN(LogBand, Log, All);

DECLARE_STATS_GROUP(TEXT("Band"), STATGROUP_Band, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("CameraToTensor"), STAT_BandCameraToTensor,
                          STATGROUP_Band, BAND_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("TextureToTensor"), STAT_BandTextureToTensor,
                          STATGROUP_Band, BAND_API);

/*
	DLL interfaces for Band / owns main interpreter
*/
class BAND_API FBandModule : public IModuleInterface {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

  /* Returns singleton object (Note: avoid calling this in shutdown phase) */
  static FBandModule& Get();
  FString GetVersion();
  BandEngine* GetEngineHandle() const;

private:
  bool InitializeInterpreter(FString ConfigPath);
  bool LoadDllFunction(FString LibraryPath);

  static void ReportError(void* UserData, const char* Format, va_list Args);
  void OnEndInvokeInternal(int32 JobId, BandStatus Status) const;

  /* Reference to interpreter. Mainly for callback propagation */
  void RegisterInterpreter(UBandInterfaceComponent* Interpreter);
  void UnregisterInterpreter(UBandInterfaceComponent* Interpreter);
  TSet<TWeakObjectPtr<UBandInterfaceComponent>> RegisteredInterpreters;

  BandEngine* EngineHandle = nullptr;
  void* LibraryHandle = nullptr;
  bool IsDllLoaded = false;

  friend class UBandTensor;
  friend class UBandModel;
  friend class UBandInterfaceComponent;
  /*
          DLL handles from Band Library
  */
  PFN_BandAddConfig BandAddConfig = nullptr;
  PFN_BandConfigBuilderCreate BandConfigBuilderCreate = nullptr;
  PFN_BandConfigBuilderDelete BandConfigBuilderDelete = nullptr;
  PFN_BandConfigCreate BandConfigCreate = nullptr;
  PFN_BandConfigDelete BandConfigDelete = nullptr;
  PFN_BandEngineCreate BandEngineCreate = nullptr;
  PFN_BandEngineCreateInputTensor BandEngineCreateInputTensor = nullptr;
  PFN_BandEngineCreateOutputTensor BandEngineCreateOutputTensor = nullptr;
  PFN_BandEngineDelete BandEngineDelete = nullptr;
  PFN_BandEngineGetNumInputTensors BandEngineGetNumInputTensors = nullptr;
  PFN_BandEngineGetNumOutputTensors BandEngineGetNumOutputTensors = nullptr;
  PFN_BandEngineGetNumWorkers BandEngineGetNumWorkers = nullptr;
  PFN_BandEngineGetWorkerDevice BandEngineGetWorkerDevice = nullptr;
  PFN_BandEngineRegisterModel BandEngineRegisterModel = nullptr;
  PFN_BandEngineRequestAsync BandEngineRequestAsync = nullptr;
  PFN_BandEngineRequestSync BandEngineRequestSync = nullptr;
  PFN_BandEngineRequestAsyncOptions BandEngineRequestAsyncOptions = nullptr;
  PFN_BandEngineRequestSyncOptions BandEngineRequestSyncOptions = nullptr;
  PFN_BandEngineWait BandEngineWait = nullptr;
  PFN_BandEngineSetOnEndRequest BandEngineSetOnEndRequest = nullptr;
  PFN_BandModelAddFromBuffer BandModelAddFromBuffer = nullptr;
  PFN_BandModelAddFromFile BandModelAddFromFile = nullptr;
  PFN_BandModelCreate BandModelCreate = nullptr;
  PFN_BandModelDelete BandModelDelete = nullptr;
  PFN_BandTensorDelete BandTensorDelete = nullptr;
  PFN_BandTensorGetBytes BandTensorGetBytes = nullptr;
  PFN_BandTensorGetData BandTensorGetData = nullptr;
  PFN_BandTensorGetNumDims BandTensorGetNumDims = nullptr;
  PFN_BandTensorGetDims BandTensorGetDims = nullptr;
  PFN_BandTensorGetName BandTensorGetName = nullptr;
  PFN_BandTensorGetQuantizationType BandTensorGetQuantizationType = nullptr;
  PFN_BandTensorGetQuantizationParams BandTensorGetQuantizationParams = nullptr;
  PFN_BandTensorGetType BandTensorGetType = nullptr;
};
