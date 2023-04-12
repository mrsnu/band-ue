// Copyright Epic Games, Inc. All Rights Reserved.

#include "Band.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "BandLibrary.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include <iostream>

#define LOCTEXT_NAMESPACE "FBandModule"
DEFINE_LOG_CATEGORY(LogBand);
DEFINE_STAT(STAT_BandCameraToTensor);
DEFINE_STAT(STAT_BandTextureToTensor);

void FBandModule::StartupModule() {
  // Get the base directory of this plugin
  FString BaseDir = IPluginManager::Get().FindPlugin("Band")->GetBaseDir();

  // Add on the relative location of the third party dll and load it
  FString LibraryPath;
#if PLATFORM_WINDOWS
  LibraryPath = FPaths::Combine(*BaseDir,
                                TEXT("Source/Band/Data/Release/band_c.dll"));
#elif PLATFORM_MAC
	LibraryPath = FString("libtensorflowlite_c.dylib");
#elif PLATFORM_ANDROID
	LibraryPath = FString("libband_c.so");
#endif // PLATFORM_WINDOWS

  UE_LOG(LogBand, Display, TEXT("Selected library path %s"), *LibraryPath);
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
  if (LoadDllFunction(LibraryPath)) {
    UE_LOG(LogBand, Display, TEXT("Successfully loaded Band library"));
  } else {
    FMessageDialog::Open(EAppMsgType::Ok,
                         LOCTEXT("BandLibrary", "Failed to load Band library"));
  }

  // TODO(dostos): Post-initialize in BeginEvent of the main BP?
  if (InitializeInterpreter("")) {
    UE_LOG(LogBand, Display, TEXT("Successfully initialized Interpreter"));
  } else {
    FMessageDialog::Open(EAppMsgType::Ok,
                         LOCTEXT("BandLibrary",
                                 "Failed to initialize Interpreter"));
  }
}

void FBandModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.

  if (EngineHandle != nullptr) {
    BandEngineDelete(EngineHandle);
    EngineHandle = nullptr;
  }

  // Free the dll handle
  FPlatformProcess::FreeDllHandle(LibraryHandle);
  LibraryHandle = nullptr;
  IsDllLoaded = false;
}

FBandModule& FBandModule::Get() {
  return *reinterpret_cast<FBandModule*>(FModuleManager::Get().
    GetModule("Band"));
}

bool FBandModule::InitializeInterpreter(FString ConfigPath) {
  FString LogDirectory = FPaths::ProjectLogDir();
  FString ProfileDirectory = FPaths::ProjectLogDir();
#if PLATFORM_ANDROID
	LogDirectory = TEXT("/sdcard/UE4Game/BandExample/BandExample/Saved/Logs/");
	ProfileDirectory = TEXT("/sdcard/UE4Game/BandExample/BandExample/Saved/Logs/");
#endif
  LogDirectory += TEXT("band_log.csv");
  ProfileDirectory += TEXT("band_profile.json");

  BandConfigBuilder* ConfigBuilder = BandConfigBuilderCreate();
  BandAddConfig(ConfigBuilder, BAND_PLANNER_LOG_PATH, 1,
                TCHAR_TO_ANSI(*LogDirectory));
  BandAddConfig(ConfigBuilder, BAND_PLANNER_SCHEDULERS, 1, kBandFixedWorker);
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
                /*count=*/1,
                30000);

  BandConfig* Config = BandConfigCreate(ConfigBuilder);

  if (Config) {
    EngineHandle = BandEngineCreate(Config);
    BandConfigDelete(Config);
  } else {
    UE_LOG(LogBand, Display, TEXT("Failed to create config"));
  }

  if (EngineHandle) {
    BandEngineSetOnEndRequest(EngineHandle,
                              [](void* UserData, int32 JobId,
                                 BandStatus Status) {
                                FBandModule* BandModule = static_cast<
                                  FBandModule*>(UserData);
                                BandModule->OnEndInvokeInternal(JobId, Status);
                              }, this);
  } else {
    UE_LOG(LogBand, Display, TEXT("Failed to create EngineHandle"));
  }

  BandConfigBuilderDelete(ConfigBuilder);

  return EngineHandle != nullptr;
}

FString FBandModule::GetVersion() {
  // TODO(dostos): Add Band version
  return FString("0.0.0");
}

BandEngine* FBandModule::GetEngineHandle() const {
  return EngineHandle;
}

#define LoadFunction(DllHandle, Function)                                                                             \
	Function = reinterpret_cast<PFN_##Function>(FPlatformProcess::GetDllExport(DllHandle, ANSI_TO_TCHAR(#Function))); \
	if (!Function)                                                                                                    \
	{                                                                                                                 \
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load " #Function));                   \
		return false;                                                                                                 \
	}

bool FBandModule::LoadDllFunction(FString LibraryPath) {
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
  IsDllLoaded = true;
  return true;
}

void FBandModule::ReportError(void* UserData, const char* Format,
                              va_list Args) {
  ANSICHAR LogMessage[256];
  FCStringAnsi::GetVarArgs(LogMessage, UE_ARRAY_COUNT(LogMessage), Format,
                           Args);

  if (FModuleManager::Get().IsModuleLoaded("Band")) {
#if WITH_EDITOR
    // Open dialog (pre editor load)
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(LogMessage));
#endif
  } else {
    UE_LOG(LogBand, Display, TEXT("%hc"), *LogMessage);
  }
}

void FBandModule::OnEndInvokeInternal(int32 JobId, BandStatus Status) const {
  // Propagate callback to actor for delegation
  for (auto& Interpreter : RegisteredInterpreters) {
    if (Interpreter.Get()) {
      Interpreter->OnEndInvokeInternal(JobId, Status);
    }
  }
}

void FBandModule::RegisterInterpreter(
    UBandInterfaceComponent* InterpreterComponent) {
  RegisteredInterpreters.Add(MakeWeakObjectPtr(InterpreterComponent));
}

void FBandModule::UnregisterInterpreter(
    UBandInterfaceComponent* InterpreterComponent) {
  for (auto& Interpreter : RegisteredInterpreters) {
    if (Interpreter.Get() == InterpreterComponent) {
      RegisteredInterpreters.Remove(Interpreter);
      break;
    }
  }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBandModule, Band)
