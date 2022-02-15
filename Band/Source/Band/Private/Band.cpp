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

#define LoadFunction(DllHandle, Function) \
    Function = reinterpret_cast<p##Function>(FPlatformProcess::GetDllExport(DllHandle, L#Function)); \
    if (!Function) { \
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load "#Function)); \
		return false; \
	}

using namespace Band;

void FBandModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!LoadDllFunction())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load Band third party library"));
	}


	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	// add custom category
	EAssetTypeCategories::Type ExampleCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Band")), FText::FromString("Band"));
	// register our custom asset with example category
	TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FBandModelTypeActions(ExampleCategory));
	AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
	// saved it here for unregister later
	CreatedAssetTypeActions.Add(Action);
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

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(LibraryHandle);
	LibraryHandle = nullptr;
}

bool FBandModule::LoadDllFunction() {
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("Band")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/BandLibrary/Win64/tensorflowlite_c.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/BandLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	LibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!LibraryHandle) {
		return false;
	}

	LoadFunction(LibraryHandle, TfLiteVersion);
	LoadFunction(LibraryHandle, TfLiteModelCreate);
	LoadFunction(LibraryHandle, TfLiteModelCreateFromFile);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsCreate);
	LoadFunction(LibraryHandle, TfLiteInterpreterOptionsDelete);
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
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBandModule, Band)
