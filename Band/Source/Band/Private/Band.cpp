// Copyright Epic Games, Inc. All Rights Reserved.

#include "Band.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include <iostream>

#define LOCTEXT_NAMESPACE "FBandModule"
DEFINE_LOG_CATEGORY(LogBand);
DEFINE_STAT(STAT_BandCameraToTensor);
DEFINE_STAT(STAT_BandTextureToTensor);

using namespace Band;

void FBandModule::StartupModule()
{
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("Band")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/Band/Data/Release/tensorflowlite_c.dll"));
#elif PLATFORM_MAC
	LibraryPath = FString("libtensorflowlite_c.dylib");
#elif PLATFORM_ANDROID
	LibraryPath = FString("libtensorflowlite_c.so");
#endif // PLATFORM_WINDOWS

	UE_LOG(LogBand, Display, TEXT("Selected library path %s"), *LibraryPath);
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (LoadDllFunction(LibraryPath))
	{
		UE_LOG(LogBand, Display, TEXT("Successfully loaded Band library"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load Band library"));
	}

	// TODO(dostos): Post-initialize in BeginEvent of the main BP?
	if (InitializeInterpreter(""))
	{
		UE_LOG(LogBand, Display, TEXT("Successfully initialized Interpreter"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to initialize Interpreter"));
	}
}

void FBandModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (InterpreterHandle != nullptr)
	{
		TfLiteInterpreterDelete(InterpreterHandle);
		InterpreterHandle = nullptr;
	}

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(LibraryHandle);
	LibraryHandle = nullptr;
	IsDllLoaded = false;
}

FBandModule& FBandModule::Get()
{
	return *reinterpret_cast<FBandModule*>(FModuleManager::Get().GetModule("Band"));
}

bool FBandModule::InitializeInterpreter(FString ConfigPath)
{
	// TODO(dostos): implement BandConfig class / replace this with default object in a project
	const FString LogDirectory = FPaths::ProjectLogDir() + TEXT("band_log.csv");
	FString DefaultConfig = FString::Format(
		ANSI_TO_TCHAR(
			"{\"allow_worksteal\":false,\"cpu_masks\":\"BIG\",\"log_path\":\"%s\",\"model_profile\":\"\",\"planner_cpu_masks\":\"BIG\",\"profile_num_runs\":3,\"profile_online\":true,\"profile_smoothing_factor\":0.1,\"profile_warmup_runs\":3,\"schedule_window_size\":5,\"schedulers\":[6],\"subgraph_preparation_type\":\"merge_unit_subgraph\"}"),
		{ *LogDirectory });
	// TODO(dostos): Android log support
#if PLATFORM_ANDROID && USE_ANDROID_FILE
	DefaultConfig = ANSI_TO_TCHAR("{\"allow_worksteal\":false,\"cpu_masks\":\"BIG\",\"log_path\":\"/sdcard/UE4Game/BandExample/BandExample/Saved/Logs/BandExample.csv\",\"model_profile\":\"\",\"planner_cpu_masks\":\"BIG\",\"profile_num_runs\":3,\"profile_online\":true,\"profile_smoothing_factor\":0.1,\"profile_warmup_runs\":3,\"schedule_window_size\":5,\"schedulers\":[6],\"subgraph_preparation_type\":\"merge_unit_subgraph\"}");
#endif
	TfLiteInterpreterOptions* InterpreterOptions =
		TfLiteInterpreterOptionsCreate();
	TfLiteInterpreterOptionsSetErrorReporter(InterpreterOptions,
		FBandModule::ReportError, this);
	TfLiteInterpreterOptionsSetOnInvokeEnd(
		InterpreterOptions, [](void* UserData, int32 JobId, TfLiteStatus Status) {
			FBandModule* BandModule = static_cast<FBandModule*>(UserData);
			BandModule->OnEndInvokeInternal(JobId, Status);
		},
		this);

	TfLiteStatus ConfigStatus = TfLiteStatus::kTfLiteError;

	if (FPaths::FileExists(ConfigPath))
	{
		UE_LOG(LogBand, Display, TEXT("Try to load config file from %s!"),
			*ConfigPath);
		ConfigStatus = TfLiteInterpreterOptionsSetConfigPath(
			InterpreterOptions, TCHAR_TO_ANSI(*ConfigPath));
	}
	else
	{
		UE_LOG(LogBand, Display, TEXT("Try to load default config"));
		ConfigStatus = TfLiteInterpreterOptionsSetConfigFile(
			InterpreterOptions, TCHAR_TO_ANSI(*DefaultConfig),
			strlen(TCHAR_TO_ANSI(*DefaultConfig)));
	}

	if (ConfigStatus == TfLiteStatus::kTfLiteOk)
	{
		InterpreterHandle = TfLiteInterpreterCreate(InterpreterOptions);
	}

	return InterpreterHandle != nullptr;
}

FString FBandModule::GetVersion()
{
	return TfLiteVersion();
}

Band::TfLiteInterpreter* FBandModule::GetInterpreterHandle() const
{
	return InterpreterHandle;
}

#define LoadFunction(DllHandle, Function)                                                                          \
	Function = reinterpret_cast<p##Function>(FPlatformProcess::GetDllExport(DllHandle, ANSI_TO_TCHAR(#Function))); \
	if (!Function)                                                                                                 \
	{                                                                                                              \
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load " #Function));                \
		return false;                                                                                              \
	}

bool FBandModule::LoadDllFunction(FString LibraryPath)
{
	LibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!LibraryHandle)
	{
		return false;
	}

	// TODO(dostos): Unload below function ptrs?
	LoadFunction(LibraryHandle, TfLiteVersion);
	LoadFunction(LibraryHandle, TfLiteModelCreate);
	LoadFunction(LibraryHandle, TfLiteModelCreateFromFile);
	LoadFunction(LibraryHandle, TfLiteModelDelete);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsCreate);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsDelete);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsSetOnInvokeEnd);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsSetConfigPath);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsSetConfigFile);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsSetErrorReporter);
	LoadFunction(LibraryHandle, TfLiteInterpreterCreate);
	LoadFunction(LibraryHandle, TfLiteInterpreterDelete);
	LoadFunction(LibraryHandle, TfLiteInterpreterRegisterModel);
	LoadFunction(LibraryHandle, TfLiteInterpreterInvokeSync);
	LoadFunction(LibraryHandle, TfLiteInterpreterInvokeAsync);
	LoadFunction(LibraryHandle, TfLiteInterpreterWait);
	LoadFunction(LibraryHandle, TfLiteInterpreterGetInputTensorCount);
	LoadFunction(LibraryHandle, TfLiteInterpreterGetOutputTensorCount);
	LoadFunction(LibraryHandle, TfLiteInterpreterAllocateInputTensor);
	LoadFunction(LibraryHandle, TfLiteInterpreterAllocateOutputTensor);
	LoadFunction(LibraryHandle, TfLiteTensorDeallocate);
	LoadFunction(LibraryHandle, TfLiteTensorType);
	LoadFunction(LibraryHandle, TfLiteTensorNumDims);
	LoadFunction(LibraryHandle, TfLiteTensorDim);
	LoadFunction(LibraryHandle, TfLiteTensorByteSize);
	LoadFunction(LibraryHandle, TfLiteTensorData);
	LoadFunction(LibraryHandle, TfLiteTensorName);
	LoadFunction(LibraryHandle, TfLiteTensorQuantizationParams);
	LoadFunction(LibraryHandle, TfLiteTensorCopyFromBuffer);
	LoadFunction(LibraryHandle, TfLiteTensorCopyToBuffer);
	IsDllLoaded = true;
	return true;
}

void FBandModule::ReportError(void* UserData, const char* Format, va_list Args)
{
	ANSICHAR LogMessage[256];
	FCStringAnsi::GetVarArgs(LogMessage, UE_ARRAY_COUNT(LogMessage), Format, Args);

	if (FModuleManager::Get().IsModuleLoaded("Band"))
	{
		#if WITH_EDITOR
		// Open dialog (pre editor load)
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(LogMessage));
		#endif
	}
	else
	{
		UE_LOG(LogBand, Display, TEXT("%s"), *LogMessage);
	}
}

void FBandModule::OnEndInvokeInternal(int32 JobId, TfLiteStatus Status) const
{
	// Propagate callback to actor for delegation
	if (Interpreter.Get())
	{
		Interpreter->OnEndInvokeInternal(JobId, Status);
	}
}

void FBandModule::RegisterInterpreter(UBandInterpreterComponent* InterpreterComponent)
{
	check(Interpreter == nullptr);
	Interpreter = MakeWeakObjectPtr(InterpreterComponent);
}

void FBandModule::UnregisterInterpreter(UBandInterpreterComponent* InterpreterComponent)
{
	check(Interpreter == InterpreterComponent);
	Interpreter = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBandModule, Band)
