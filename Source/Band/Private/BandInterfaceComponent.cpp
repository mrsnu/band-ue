// Fill out your copyright notice in the Description page of Project Settings.


#include "BandInterfaceComponent.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "Band.h"
#include "BandTensorUtil.h"


void UBandInterfaceComponent::BeginPlay() {
  // Reserve enough amount to avoid run-time realloc
  JobToModel.reserve(1000);
  FBandModule::Get().RegisterInterpreter(this);

  Super::BeginPlay();
}

void UBandInterfaceComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  FBandModule::Get().UnregisterInterpreter(this);
  Super::EndPlay(EndPlayReason);
}

int32 UBandInterfaceComponent::GetInputTensorCount(
    UPARAM(ref) const UBandModel* Model) {
  // Skip model registration check (lazily register in get handle)
  return FBandModule::Get().BandEngineGetNumInputTensors(
      GetHandle(), Model->GetHandle());
}

int32 UBandInterfaceComponent::GetOutputTensorCount(
    UPARAM(ref) const UBandModel* Model) {
  // Skip model registration check (lazily register in get handle)
  return FBandModule::Get().BandEngineGetNumOutputTensors(
      GetHandle(), Model->GetHandle());
}

UBandTensor* UBandInterfaceComponent::AllocateInputTensor(
    UPARAM(ref) const UBandModel* Model, int32 InputIndex) {
  // Skip model registration check (lazily register in get handle)
  UBandTensor* Tensor = NewObject<UBandTensor>();
  Tensor->Initialize(
      FBandModule::Get().BandEngineCreateInputTensor(
          GetHandle(), Model->GetHandle(), InputIndex));
  return Tensor;
}

UBandTensor* UBandInterfaceComponent::AllocateOutputTensor(
    UPARAM(ref) const UBandModel* Model, int32 OutputIndex) {
  // Skip model registration check (lazily register in get handle)
  UBandTensor* Tensor = NewObject<UBandTensor>();
  Tensor->Initialize(
      FBandModule::Get().BandEngineCreateOutputTensor(
          GetHandle(), Model->GetHandle(), OutputIndex));
  return Tensor;
}

void UBandInterfaceComponent::InvokeSyncSingleIO(
    const UBandModel* Model, UBandTensor* InputTensor,
    UBandTensor* OutputTensor) {
  InvokeSync(Model, {InputTensor}, {OutputTensor});
}

int32 UBandInterfaceComponent::InvokeAsyncSingleInput(
    const UBandModel* Model, UBandTensor* InputTensor) {
  return InvokeAsync(Model, {InputTensor});
}

void UBandInterfaceComponent::InvokeSync(
    UPARAM(ref) const UBandModel* Model,
    UPARAM(ref) TArray<UBandTensor*> InputTensors,
    UPARAM(ref) TArray<UBandTensor*> OutputTensors, int DeviceFlag) {
  if (Model->IsRegistered() && (InputTensors.Num() == 0 || OutputTensors.Num()
                                == 0)) {
    UE_LOG(LogBand, Error,
           TEXT(
             "InvokeSync: Failed to invoke. Please check that the input/output tensors are non null"
           ));
    // FBandModule::Get().BandEngineRequestSyncOnWorker(GetHandle(), Model->GetHandle(), DeviceFlag, nullptr, nullptr);	
  } else if (Model->IsRegistered() && GetInputTensorCount(Model) == InputTensors
             .Num() && GetOutputTensorCount(Model) == OutputTensors.Num()) {
    FBandModule::Get().BandEngineRequestSyncOptions(
        GetHandle(), Model->GetHandle(), {DeviceFlag, false, -1, -1},
        BandTensorUtil::TensorsFromTArray(InputTensors).GetData(),
        BandTensorUtil::TensorsFromTArray(OutputTensors).GetData());
  } else {
    UE_LOG(LogBand, Error,
           TEXT(
             "InvokeSync: Failed to invoke. Model registered %d Input Count (Expected %d, Given %d) Output Count (Expected %d, Given %d)"
           ),
           Model->IsRegistered(), GetInputTensorCount(Model),
           InputTensors.Num(), GetOutputTensorCount(Model),
           OutputTensors.Num());
  }
}

int32 UBandInterfaceComponent::InvokeAsync(
    UPARAM(ref) const UBandModel* Model,
    UPARAM(ref) TArray<UBandTensor*> InputTensors, int DeviceFlag) {
  if (Model->IsRegistered() && InputTensors.Num() == 0) {
    const int32 JobId = FBandModule::Get().BandEngineRequestAsyncOptions(
        GetHandle(), Model->GetHandle(), {DeviceFlag, false, -1, -1}, nullptr);
    JobToModel[JobId] = Model->GetHandle();
    return JobId;
  } else if (Model->IsRegistered() && GetInputTensorCount(Model) == InputTensors
             .Num()) {
    const int32 JobId = FBandModule::Get().BandEngineRequestAsyncOptions(
        GetHandle(), Model->GetHandle(), {DeviceFlag, false, -1, -1},
        BandTensorUtil::TensorsFromTArray(InputTensors).GetData());
    JobToModel[JobId] = Model->GetHandle();
    return JobId;
  } else {
    UE_LOG(LogBand, Error,
           TEXT(
             "InvokeSync: Failed to invoke. Model registered %d Input Count (Expected %d, Given %d)"
           ),
           Model->IsRegistered(), GetInputTensorCount(Model),
           InputTensors.Num());
    return -1;
  }
}

EBandStatus UBandInterfaceComponent::Wait(int32 JobId,
                                          UPARAM(ref) TArray<UBandTensor*>
                                          OutputTensors) {
  // TODO(dostos): Add `GetJob(int32 jobId)` to C API?
  const int32 OutputTensorCount = FBandModule::Get().
      BandEngineGetNumOutputTensors(GetHandle(), JobToModel[JobId]);
  if (OutputTensors.Num() == OutputTensorCount) {
    return static_cast<EBandStatus>(FBandModule::Get().BandEngineWait(
        GetHandle(), JobId,
        BandTensorUtil::TensorsFromTArray(OutputTensors).GetData(),
        OutputTensorCount));
  } else {
    UE_LOG(LogBand, Error,
           TEXT(
             "InvokeSync: Failed to invoke. Output Count (Expected %d, Given %d)"
           ),
           OutputTensors.Num(), OutputTensorCount);
    return EBandStatus::Error;
  }
}

int32 UBandInterfaceComponent::GetNumWorkers() const {
  return FBandModule::Get().BandEngineGetNumWorkers(GetHandle());
}

int32 UBandInterfaceComponent::GetWorkerDeviceType(int Index) const {
  return FBandModule::Get().BandEngineGetWorkerDevice(GetHandle(), Index);
}

void UBandInterfaceComponent::OnEndInvokeInternal(
    int32 JobId, BandStatus Status) const {
  AsyncTask(ENamedThreads::GameThread, [&, JobId, Status]() {
    OnEndInvokeDynamic.Broadcast(JobId, BandEnum::ToBandStatus(Status));
    JobToModel.erase(JobId);
  });
  OnEndInvoke.Broadcast(JobId, BandEnum::ToBandStatus(Status));
}

BandEngine* UBandInterfaceComponent::GetHandle() const {
  return FBandModule::Get().GetEngineHandle();
}
