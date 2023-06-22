// Copyright Epic Games, Inc. All Rights Reserved.

#include "Band.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "libband.h"

#include "Core.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#include <iostream>

#include "BandTensorUtil.h"

#define LOCTEXT_NAMESPACE "FBandModule"
DEFINE_LOG_CATEGORY(LogBand);

class FBandModuleImpl {
public:
  ~FBandModuleImpl();
  bool Initialize();

private:
  friend class FBandModule;
  bool InitializeEngine(FString ConfigPath);
  bool LoadDllFunction(FString LibraryPath);
  // Function pointer registered to BandEngine
  void OnEndInvokeInternal(int32 JobId, BandStatus Status) const;

  TSet<UBandSubSystem*> RegisteredSubSystems;

  BandEngine *EngineHandle = nullptr;
  void *LibraryHandle = nullptr;

  /*
          DLL handles from Band Library
  */
  /* engine internal functions */
  PFN_BandAddConfig BandAddConfig = nullptr;
  PFN_BandConfigBuilderCreate BandConfigBuilderCreate = nullptr;
  PFN_BandConfigBuilderDelete BandConfigBuilderDelete = nullptr;
  PFN_BandConfigCreate BandConfigCreate = nullptr;
  PFN_BandConfigDelete BandConfigDelete = nullptr;
  PFN_BandEngineCreate BandEngineCreate = nullptr;
  PFN_BandEngineDelete BandEngineDelete = nullptr;
  PFN_BandEngineSetOnEndRequest BandEngineSetOnEndRequest = nullptr;

  PFN_BandEngineCreateInputTensor BandEngineCreateInputTensor = nullptr;
  PFN_BandEngineCreateOutputTensor BandEngineCreateOutputTensor = nullptr;
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

FBandModuleImpl::~FBandModuleImpl() {
  if (EngineHandle != nullptr) {
    BandEngineDelete(EngineHandle);
    EngineHandle = nullptr;
  }

  if (LibraryHandle != nullptr) {
    // Free the dll handle
    FPlatformProcess::FreeDllHandle(LibraryHandle);
  }
}

bool FBandModuleImpl::Initialize() {
  // Get the base directory of this plugin
  FString BaseDir = IPluginManager::Get().FindPlugin("Band")->GetBaseDir();

  // Add on the relative location of the third party dll and load it
  FString LibraryPath;
#if PLATFORM_WINDOWS
  LibraryPath = FPaths::Combine(
      *BaseDir, TEXT("Source/Band/Library/Release/lib/band_c.dll"));
#elif PLATFORM_MAC
  LibraryPath = FString("libband_c.dylib");
#elif PLATFORM_ANDROID
  LibraryPath = FString("libband_c.so");
#endif // PLATFORM_WINDOWS

  UE_LOG(LogBand, Display, TEXT("Selected library path %s"), *LibraryPath);
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module
  if (!LoadDllFunction(LibraryPath)) {
    FMessageDialog::Open(EAppMsgType::Ok,
                         LOCTEXT("BandLibrary", "Failed to load Band library"));
  }

  if (!InitializeEngine("")) {
    FMessageDialog::Open(
        EAppMsgType::Ok,
        LOCTEXT("BandLibrary", "Failed to initialize BandEngine"));
    return false;
  }

  return true;
}

void FBandModule::StartupModule() {
  Impl = new FBandModuleImpl();
  if (!Impl->Initialize()) {
    delete Impl;
    Impl = nullptr;
  }
}

void FBandModule::ShutdownModule() {
  if (Impl != nullptr) {
    delete Impl;
    Impl = nullptr;
  }
}

bool FBandModuleImpl::InitializeEngine(FString ConfigPath) {
  FString LogDirectory = FPaths::ProjectLogDir();
  FString ProfileDirectory = FPaths::ProjectLogDir();
#if PLATFORM_ANDROID
  LogDirectory = TEXT("/sdcard/UE4Game/BandExample/BandExample/Saved/Logs/");
  ProfileDirectory =
      TEXT("/sdcard/UE4Game/BandExample/BandExample/Saved/Logs/");
#endif
  LogDirectory += TEXT("band_log.csv");
  ProfileDirectory += TEXT("band_profile.json");

  BandConfigBuilder *ConfigBuilder = BandConfigBuilderCreate();
  BandAddConfig(ConfigBuilder, BAND_PLANNER_LOG_PATH, 1,
                TCHAR_TO_ANSI(*LogDirectory));
  BandAddConfig(ConfigBuilder, BAND_PLANNER_SCHEDULERS, 1,
                kBandHeterogeneousEarliestFinishTimeReserved);
  BandAddConfig(ConfigBuilder, BAND_CPU_MASK, /*count=*/1, kBandAll);
  BandAddConfig(ConfigBuilder, BAND_PLANNER_CPU_MASK, /*count=*/1,
                kBandPrimary);
  BandAddConfig(ConfigBuilder, BAND_WORKER_WORKERS, /*count=*/3, kBandCPU,
                kBandNPU, kBandDSP);
  BandAddConfig(ConfigBuilder, BAND_WORKER_CPU_MASKS, /*count=*/3, kBandBig,
                kBandBig, kBandBig);
  BandAddConfig(ConfigBuilder, BAND_WORKER_NUM_THREADS, /*count=*/3, 1, 1, 1);
  BandAddConfig(ConfigBuilder, BAND_PROFILE_SMOOTHING_FACTOR, /*count=*/1,
                0.1f);
  BandAddConfig(ConfigBuilder, BAND_PROFILE_DATA_PATH, /*count=*/1,
                TCHAR_TO_ANSI(*ProfileDirectory));
  BandAddConfig(ConfigBuilder, BAND_PROFILE_ONLINE, /*count=*/1, true);
  BandAddConfig(ConfigBuilder, BAND_PROFILE_NUM_WARMUPS, /*count=*/1, 1);
  BandAddConfig(ConfigBuilder, BAND_PROFILE_NUM_RUNS, /*count=*/1, 1);
  BandAddConfig(ConfigBuilder, BAND_WORKER_AVAILABILITY_CHECK_INTERVAL_MS,
                /*count=*/1, 30000);

  BandConfig *Config = BandConfigCreate(ConfigBuilder);

  if (Config) {
    EngineHandle = BandEngineCreate(Config);
    BandConfigDelete(Config);
  } else {
    UE_LOG(LogBand, Display, TEXT("Failed to create config"));
  }

  if (EngineHandle) {
    BandEngineSetOnEndRequest(
        EngineHandle,
        [](void *UserData, int32 JobId, BandStatus Status) {
          FBandModuleImpl *BandModule =
              static_cast<FBandModuleImpl *>(UserData);
          BandModule->OnEndInvokeInternal(JobId, Status);
        },
        this);
  } else {
    UE_LOG(LogBand, Display, TEXT("Failed to create EngineHandle"));
  }

  BandConfigBuilderDelete(ConfigBuilder);

  return EngineHandle != nullptr;
}

#define LoadFunction(DllHandle, Function)                                      \
  Function = reinterpret_cast<PFN_##Function>(                                 \
      FPlatformProcess::GetDllExport(DllHandle, ANSI_TO_TCHAR(#Function)));    \
  if (!Function) {                                                             \
    FMessageDialog::Open(EAppMsgType::Ok,                                      \
                         LOCTEXT("BandLibrary", "Failed to load " #Function)); \
    return false;                                                              \
  }

bool FBandModuleImpl::LoadDllFunction(FString LibraryPath) {
  LibraryHandle = !LibraryPath.IsEmpty()
                      ? FPlatformProcess::GetDllHandle(*LibraryPath)
                      : nullptr;

  if (!LibraryHandle) {
    return false;
  }

  // TODO(dostos): Unload below function ptrs?
  LoadFunction(LibraryHandle, BandAddConfig);
  LoadFunction(LibraryHandle, BandConfigBuilderCreate);
  LoadFunction(LibraryHandle, BandConfigBuilderDelete);
  LoadFunction(LibraryHandle, BandConfigCreate);
  LoadFunction(LibraryHandle, BandConfigDelete);
  LoadFunction(LibraryHandle, BandEngineCreate);
  LoadFunction(LibraryHandle, BandEngineCreateInputTensor);
  LoadFunction(LibraryHandle, BandEngineCreateOutputTensor);
  LoadFunction(LibraryHandle, BandEngineDelete);
  LoadFunction(LibraryHandle, BandEngineGetNumInputTensors);
  LoadFunction(LibraryHandle, BandEngineGetNumOutputTensors);
  LoadFunction(LibraryHandle, BandEngineGetNumWorkers);
  LoadFunction(LibraryHandle, BandEngineGetWorkerDevice);
  LoadFunction(LibraryHandle, BandEngineRegisterModel);
  LoadFunction(LibraryHandle, BandEngineRequestAsync);
  LoadFunction(LibraryHandle, BandEngineRequestSync);
  LoadFunction(LibraryHandle, BandEngineRequestAsyncOptions);
  LoadFunction(LibraryHandle, BandEngineRequestSyncOptions);
  LoadFunction(LibraryHandle, BandEngineWait);
  LoadFunction(LibraryHandle, BandEngineSetOnEndRequest);
  LoadFunction(LibraryHandle, BandModelAddFromBuffer);
  LoadFunction(LibraryHandle, BandModelAddFromFile);
  LoadFunction(LibraryHandle, BandModelCreate);
  LoadFunction(LibraryHandle, BandModelDelete);
  LoadFunction(LibraryHandle, BandTensorDelete);
  LoadFunction(LibraryHandle, BandTensorGetBytes);
  LoadFunction(LibraryHandle, BandTensorGetData);
  LoadFunction(LibraryHandle, BandTensorGetNumDims);
  LoadFunction(LibraryHandle, BandTensorGetDims);
  LoadFunction(LibraryHandle, BandTensorGetName);
  LoadFunction(LibraryHandle, BandTensorGetQuantizationType);
  LoadFunction(LibraryHandle, BandTensorGetQuantizationParams);
  LoadFunction(LibraryHandle, BandTensorGetType);
  return true;
}

void FBandModuleImpl::OnEndInvokeInternal(int32 JobId,
                                          BandStatus Status) const {
  // Propagate callback to actor for delegation
  for (auto &BandEngine : RegisteredSubSystems) {
      UE_LOG(LogBand, Display, TEXT("OnEndInvokeInternal"));
      BandEngine->OnEndInvokeInternal(JobId, Status);
  }
}

void FBandModule::RegisterSubSystem(UBandSubSystem *BandEngineComponent) {
  UE_LOG(LogBand, Display, TEXT("RegisterSubSystem"));
  Impl->RegisteredSubSystems.Add(BandEngineComponent);
}

void FBandModule::UnregisterSubSystem(UBandSubSystem *BandEngineComponent) {
  UE_LOG(LogBand, Display, TEXT("UnregisterSubSystem"));
  for (auto &BandEngine : Impl->RegisteredSubSystems) {
    if (BandEngine == BandEngineComponent) {
      Impl->RegisteredSubSystems.Remove(BandEngine);
      break;
    }
  }
}

TArray<BandTensor *>
FBandModule::TensorsFromTArray(TArray<UBandTensor *> Tensors) {
  TArray<BandTensor *> OutTensors;

  for (int i = 0; i < Tensors.Num(); i++) {
    if (Tensors[i]) {
      OutTensors.Push(Tensors[i]->Handle);
    } else {
      UE_LOG(LogBand, Error, TEXT("Try to access null tensor."));
    }
  }

  return OutTensors;
}

FBandModule &FBandModule::Get() {
  return *reinterpret_cast<FBandModule *>(
      FModuleManager::Get().GetModule("Band"));
}

FString FBandModule::GetVersion() {
  // TODO: Get version from DLL
  return FString("v1.0.0");
}

int FBandModule::GetNumWorkers() {
  return Impl->BandEngineGetNumWorkers(Impl->EngineHandle);
}

EBandDeviceType FBandModule::GetDeviceType(int WorkerIndex) {
  return static_cast<EBandDeviceType>(
      Impl->BandEngineGetWorkerDevice(Impl->EngineHandle, WorkerIndex));
}

void FBandModule::RegisterModel(const UBandModel *Model) {
  std::unique_lock<std::mutex> RegisterLock(Model->RegisterMutex);
  if (Model->ModelBinary.Num() && Model->Handle == nullptr) {
    Model->Handle = Impl->BandModelCreate();
    if (Impl->BandModelAddFromBuffer(Model->Handle, kBandTfLite,
                                     Model->ModelBinary.GetData(),
                                     Model->ModelBinary.Num()) == kBandOk &&
        Impl->BandEngineRegisterModel(Impl->EngineHandle, Model->Handle) ==
            kBandOk) {
    } else {
      Impl->BandModelDelete(Model->Handle);
    }
  }
}

void FBandModule::DeleteModel(UBandModel *Model) {
  if (Model && Model->GetHandle()) {
    Impl->BandModelDelete(Model->GetHandle());
    Model->Handle = nullptr;
  }
}

UBandTensor *FBandModule::CreateInputTensor(const UBandModel *Model,
                                            int32 Index) {
  UBandTensor *Tensor = NewObject<UBandTensor>();
  Tensor->Initialize(Impl->BandEngineCreateInputTensor(
      Impl->EngineHandle, Model->GetHandle(), Index));
  return Tensor;
}

UBandTensor *FBandModule::CreateOutputTensor(const UBandModel *Model,
                                             int32 Index) {
  UBandTensor *Tensor = NewObject<UBandTensor>();
  Tensor->Initialize(Impl->BandEngineCreateOutputTensor(
      Impl->EngineHandle, Model->GetHandle(), Index));
  return Tensor;
}

int FBandModule::GetInputTensorCount(const UBandModel *Model) {
  return Impl->BandEngineGetNumInputTensors(Impl->EngineHandle,
                                            Model->GetHandle());
}

int FBandModule::GetOutputTensorCount(const UBandModel *Model) {
  return Impl->BandEngineGetNumOutputTensors(Impl->EngineHandle,
                                             Model->GetHandle());
}

int FBandModule::RequestAsync(const UBandModel *Model,
                              const TArray<UBandTensor *> &Inputs) {
  // by default, we require a callback to get notified when the request is done
  BandRequestOption Options{-1, true, -1, -1};
  return Impl->BandEngineRequestAsyncOptions(
      Impl->EngineHandle, Model->GetHandle(), Options,
      TensorsFromTArray(Inputs).GetData());
}

void FBandModule::RequestSync(const UBandModel *Model,
                              const TArray<UBandTensor *> &Inputs,
                              const TArray<UBandTensor *> &Outputs) {
  Impl->BandEngineRequestSync(Impl->EngineHandle, Model->GetHandle(),
                              TensorsFromTArray(Inputs).GetData(),
                              TensorsFromTArray(Outputs).GetData());
}

int FBandModule::RequestAsyncOptions(const UBandModel *Model,
                                     const TArray<UBandTensor *> &Inputs,
                                     int32 WorkerIndex) {
  BandRequestOption Options{WorkerIndex, true, -1, -1};
  return Impl->BandEngineRequestAsyncOptions(
      Impl->EngineHandle, Model->GetHandle(), Options,
      TensorsFromTArray(Inputs).GetData());
}

void FBandModule::RequestSyncOptions(const UBandModel *Model,
                                     const TArray<UBandTensor *> &Inputs,
                                     const TArray<UBandTensor *> &Outputs,
                                     int32 WorkerIndex) {
  BandRequestOption Options{WorkerIndex, false, -1, -1};
  Impl->BandEngineRequestSyncOptions(Impl->EngineHandle, Model->GetHandle(),
                                     Options,
                                     TensorsFromTArray(Inputs).GetData(),
                                     TensorsFromTArray(Outputs).GetData());
}

EBandStatus FBandModule::GetOutputs(int32 JobId,
                                    const TArray<UBandTensor *> &Outputs) {
  BandStatus status =
      Impl->BandEngineWait(Impl->EngineHandle, JobId,
                           TensorsFromTArray(Outputs).GetData(), Outputs.Num());
  return static_cast<EBandStatus>(status);
}

void FBandModule::DeleteTensor(UBandTensor *Tensor) {
  Impl->BandTensorDelete(Tensor->Handle);
}

int FBandModule::GetTensorBytes(UBandTensor *Tensor) {
  return Impl->BandTensorGetBytes(Tensor->Handle);
}

const uint8 *FBandModule::GetTensorData(UBandTensor *Tensor) {
  return static_cast<const uint8 *>(Impl->BandTensorGetData(Tensor->Handle));
}

const int *FBandModule::GetTensorDims(UBandTensor *Tensor) {
  return Impl->BandTensorGetDims(Tensor->Handle);
}

int FBandModule::GetTensorNumDims(UBandTensor *Tensor) {
  return Impl->BandTensorGetNumDims(Tensor->Handle);
}

FString FBandModule::GetTensorName(UBandTensor *Tensor) {
  return Impl->BandTensorGetName(Tensor->Handle);
}

EBandTensorDataType FBandModule::GetTensorType(UBandTensor *Tensor) {
  return static_cast<EBandTensorDataType>(
      Impl->BandTensorGetType(Tensor->Handle));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBandModule, Band)
