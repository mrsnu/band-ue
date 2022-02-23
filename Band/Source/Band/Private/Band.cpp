// Copyright Epic Games, Inc. All Rights Reserved.

#include "Band.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
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
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Data/Release/tensorflowlite_c.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Data/Release/tensorflowlite_c.dylib"));
#elif PLATFORM_ANDROID
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Data/Release/tensorflowlite_c.so"));
#endif // PLATFORM_WINDOWS

	UE_LOG(LogTemp, Display, TEXT("Band: Selected library path %s"), *LibraryPath);
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (LoadDllFunction(LibraryPath))
	{	
		FString ConfigPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/BandLibrary/Data/runtime_config.json"));

		if (FPaths::FileExists(ConfigPath)) {
			TfLiteInterpreterOptions* InterpreterOptions = TfLiteInterpreterOptionsCreate();
			TfLiteInterpreterOptionsSetErrorReporter(InterpreterOptions, FBandModule::ReportError, this);
			TfLiteStatus success = TfLiteInterpreterOptionsSetConfigPath(InterpreterOptions, TCHAR_TO_ANSI(*ConfigPath));
			Interpreter = TfLiteInterpreterCreate(InterpreterOptions);
		}
		else {
			UE_LOG(LogTemp, Display, TEXT("File not exists!"));
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BandLibrary", "Failed to load Band third party library"));
		}
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

	if (Interpreter != nullptr) {
		TfLiteInterpreterDelete(Interpreter);
		Interpreter = nullptr;
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

FString FBandModule::GetVersion()
{
	return TfLiteVersion();
}

int32 FBandModule::GetInputTensorCount(UBandModel* Model)
{
	return TfLiteInterpreterGetInputTensorCount(Interpreter, Model->GetModelHandle());
}

int32 FBandModule::GetOutputTensorCount(UBandModel* Model)
{
	return TfLiteInterpreterGetOutputTensorCount(Interpreter, Model->GetModelHandle());
}

UBandTensor* FBandModule::AllocateInputTensor(UBandModel* Model, int32 InputIndex)
{
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->TensorHandle = TfLiteInterpreterAllocateInputTensor(Interpreter, Model->GetModelHandle(), InputIndex);
	return Tensor;
}

UBandTensor* FBandModule::AllocateOutputTensor(UBandModel* Model, int32 OutputIndex)
{
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->TensorHandle = TfLiteInterpreterAllocateOutputTensor(Interpreter, Model->GetModelHandle(), OutputIndex);
	return Tensor;
}

TArray<TfLiteTensor*> FBandModule::TensorsFromTArray(TArray<UBandTensor*> Tensors)
{
	TArray<TfLiteTensor*> OutTensors;

	for (int i = 0; i < Tensors.Num(); i++) {
		OutTensors.Push(Tensors[i]->TensorHandle);
	}

	return OutTensors;
}

void FBandModule::InvokeSync(UBandModel* Model, TArray<UBandTensor*> InputTensors, TArray<UBandTensor*> OutputTensors)
{
	TfLiteInterpreterInvokeSync(Interpreter, Model->GetModelHandle(), TensorsFromTArray(InputTensors).GetData(), TensorsFromTArray(OutputTensors).GetData());
}

int32 FBandModule::InvokeAsync(UBandModel* Model, TArray<UBandTensor*> InputTensors)
{
	return TfLiteInterpreterInvokeAsync(Interpreter, Model->GetModelHandle(), TensorsFromTArray(InputTensors).GetData());
}

EBandStatus FBandModule::Wait(int32 JobHandle, TArray<UBandTensor*> OutputTensors)
{
	return EBandStatus(TfLiteInterpreterWait(Interpreter, JobHandle, TensorsFromTArray(OutputTensors).GetData()));
}

Band::TfLiteInterpreter* FBandModule::GetInterpreter()
{
	return Interpreter;
}


#define LoadFunction(DllHandle, Function) \
    Function = reinterpret_cast<p##Function>(FPlatformProcess::GetDllExport(DllHandle, ANSI_TO_TCHAR(#Function))); \
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
	LoadFunction(LibraryHandle, TfLiteModelDelete);
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