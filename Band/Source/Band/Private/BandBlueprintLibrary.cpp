#include "BandBlueprintLibrary.h"
#include "Band.h"
#include "BandModel.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

int32 UBandBlueprintLibrary::GetInputTensorCount(UBandModel* Model)
{
	return FBandModule::Get().GetInputTensorCount(Model);
}

int32 UBandBlueprintLibrary::GetOutputTensorCount(UBandModel* Model)
{
	return FBandModule::Get().GetOutputTensorCount(Model);
}

UBandTensor* UBandBlueprintLibrary::AllocateInputTensor(UBandModel* Model, int32 InputIndex)
{
	return FBandModule::Get().AllocateInputTensor(Model, InputIndex);
}

UBandTensor* UBandBlueprintLibrary::AllocateOutputTensor(UBandModel* Model, int32 OutputIndex)
{
	return FBandModule::Get().AllocateInputTensor(Model, OutputIndex);
}

void UBandBlueprintLibrary::InvokeSync(UBandModel* Model, const TArray<UBandTensor*>& InputTensors, TArray<UBandTensor*>& OutputTensors)
{
	FBandModule::Get().InvokeSync(Model, InputTensors, OutputTensors);
}

int32 UBandBlueprintLibrary::InvokeAsync(UBandModel* Model, const TArray<UBandTensor*>& InputTensors, TArray<UBandTensor*>& OutputTensors)
{
	return FBandModule::Get().InvokeAsync(Model, InputTensors);
}

EBandStatus UBandBlueprintLibrary::Wait(int32 JobHandle, TArray<UBandTensor*>& OutputTensors)
{
	return FBandModule::Get().Wait(JobHandle, OutputTensors);
}
