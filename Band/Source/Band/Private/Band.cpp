// Copyright Epic Games, Inc. All Rights Reserved.

#include "Band.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"
#include "BandModelTypeActions.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Interfaces/IPluginManager.h"

#include <iostream>

#define LOCTEXT_NAMESPACE "FBandModule"

using namespace Band;

void FBandModule::StartupModule()
{
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("Band")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/x64/Release/tensorflowlite_c.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/BandLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (LoadDllFunction(LibraryPath))
	{	
		FString ConfigPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Data/runtime_config.json"));

		TfLiteInterpreterOptions* InterpreterOptions = TfLiteInterpreterOptionsCreate();
		TfLiteInterpreterOptionsSetErrorReporter(InterpreterOptions, FBandModule::ReportError, this);
		TfLiteStatus success = TfLiteInterpreterOptionsSetConfigPath(InterpreterOptions, TCHAR_TO_ANSI(*ConfigPath));
		Interpreter = TfLiteInterpreterCreate(InterpreterOptions);

		// TODO(dostos): Separate editor logic?
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		// add custom category
		EAssetTypeCategories::Type ExampleCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Band")), FText::FromString("Band"));
		// register our custom asset with example category
		TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FBandModelTypeActions(ExampleCategory));
		AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
		// saved it here for unregister later
		CreatedAssetTypeActions.Add(Action);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load Band third party library"));
	}
}

void FBandModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();

	if (Interpreter != nullptr) {
		TfLiteInterpreterDelete(Interpreter);
		Interpreter = nullptr;
	}

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(LibraryHandle);
	LibraryHandle = nullptr;
	IsDllLoaded = false;
}

#define LoadFunction(DllHandle, Function) \
    Function = reinterpret_cast<p##Function>(FPlatformProcess::GetDllExport(DllHandle, L#Function)); \
    if (!Function) { \
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load "#Function)); \
		return false; \
	}

bool FBandModule::LoadDllFunction(FString LibraryPath) {
	LibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!LibraryHandle) {
		return false;
	}

	// TODO(dostos): Unload below function ptrs?
	LoadFunction(LibraryHandle, TfLiteVersion);
	LoadFunction(LibraryHandle, TfLiteModelCreate);
	LoadFunction(LibraryHandle, TfLiteModelCreateFromFile);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsCreate);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsDelete);
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
	FBandModule* BandModule = static_cast<FBandModule*>(UserData);

	ANSICHAR LogMessage[256];
	FCStringAnsi::GetVarArgs(LogMessage, UE_ARRAY_COUNT(LogMessage), Format, Args);

	if (FModuleManager::Get().IsModuleLoaded("Band")) 
	{
		// Open dialog (pre editor load)
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(LogMessage));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBandModule, Band)
