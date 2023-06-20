// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "BandEnum.h"
#include "BandSubsystem.h"
#include "Modules/ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(LogBand, Log, All);

/*
 * Main entry point for the Band library. This module is responsible for loading
 * engine and model configurations, and propagating events to subsystems.
 *
 * C++ code should use the FBandModule::Get() singleton to access the module and
 * C API functions.
 */
class FBandModuleImpl;
class BAND_API FBandModule : public IModuleInterface {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

  /* Returns singleton object (Note: avoid calling this in shutdown phase) */
  static FBandModule &Get();
  FString GetVersion();

  int GetNumWorkers();
  EBandDeviceType GetDeviceType(int WorkerIndex);

  /* Model interface */
  void RegisterModel(const UBandModel *Model);
  void DeleteModel(UBandModel *Model);

  /* Engine interface */
  UBandTensor *CreateInputTensor(const UBandModel *Model, int32 Index);
  UBandTensor *CreateOutputTensor(const UBandModel *Model, int32 Index);
  int GetInputTensorCount(const UBandModel *Model);
  int GetOutputTensorCount(const UBandModel *Model);
  int RequestAsync(const UBandModel *Model,
                   const TArray<UBandTensor *> &Inputs);
  void RequestSync(const UBandModel *Model, const TArray<UBandTensor *> &Inputs,
                   const TArray<UBandTensor *> &Outputs);
  int RequestAsyncOptions(const UBandModel *Model,
                          const TArray<UBandTensor *> &Inputs,
                          int32 WorkerIndex);
  void RequestSyncOptions(const UBandModel *Model,
                          const TArray<UBandTensor *> &Inputs,
                          const TArray<UBandTensor *> &Outputs,
                          int32 WorkerIndex);
  EBandStatus GetOutputs(int32 JobId, const TArray<UBandTensor *> &Outputs);

  /* Tensor interface */
  void DeleteTensor(UBandTensor *Tensor);
  int GetTensorBytes(UBandTensor *Tensor);
  const uint8 *GetTensorData(UBandTensor *Tensor);
  const int *GetTensorDims(UBandTensor *Tensor);
  int GetTensorNumDims(UBandTensor *Tensor);
  FString GetTensorName(UBandTensor *Tensor);
  EBandTensorDataType GetTensorType(UBandTensor *Tensor);

  // Reference to subsystems, mainly for propagating events
  void RegisterSubSystem(UBandSubSystem *Interpreter);
  void UnregisterSubSystem(UBandSubSystem *Interpreter);

private:
  TArray<BandTensor *> TensorsFromTArray(TArray<UBandTensor *> Tensors);
  FBandModuleImpl *Impl = nullptr;
};
