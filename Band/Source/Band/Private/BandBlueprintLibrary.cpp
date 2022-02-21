#include "BandBlueprintLibrary.h"
#include "Band.h"
#include "BandModel.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

int32 UBandBlueprintLibrary::GetInputTensorCount(UPARAM(ref) UBandModel* Model)
{
	return FBandModule::Get().GetInputTensorCount(Model);
}

int32 UBandBlueprintLibrary::GetOutputTensorCount(UPARAM(ref) UBandModel* Model)
{
	return FBandModule::Get().GetOutputTensorCount(Model);
}

UBandTensor* UBandBlueprintLibrary::AllocateInputTensor(UPARAM(ref) UBandModel* Model, int32 InputIndex)
{
	return FBandModule::Get().AllocateInputTensor(Model, InputIndex);
}

UBandTensor* UBandBlueprintLibrary::AllocateOutputTensor(UPARAM(ref) UBandModel* Model, int32 OutputIndex)
{
	return FBandModule::Get().AllocateOutputTensor(Model, OutputIndex);
}

void UBandBlueprintLibrary::InvokeSync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	FBandModule::Get().InvokeSync(Model, InputTensors, OutputTensors);
}

int32 UBandBlueprintLibrary::InvokeAsync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	return FBandModule::Get().InvokeAsync(Model, InputTensors);
}

EBandStatus UBandBlueprintLibrary::Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	return FBandModule::Get().Wait(JobHandle, OutputTensors);
}

EBandTensorType UBandBlueprintLibrary::GetTensorType(UBandTensor* Tensor)
{
	return Tensor->Type();
}

int32 UBandBlueprintLibrary::NumDims(UBandTensor* Tensor)
{
	return Tensor->NumDims();
}

int32 UBandBlueprintLibrary::ByteSize(UBandTensor* Tensor)
{
	return Tensor->ByteSize();
}

TArray<uint8> UBandBlueprintLibrary::Data(UBandTensor* Tensor)
{
	return Tensor->Data();
}

FString UBandBlueprintLibrary::Name(UBandTensor* Tensor)
{
	return Tensor->Name();
}

EBandStatus UBandBlueprintLibrary::CopyFromBuffer(UBandTensor* Tensor, TArray<uint8> Buffer)
{
	return Tensor->CopyFromBuffer(Buffer);
}

EBandStatus UBandBlueprintLibrary::CopyToBuffer(UBandTensor* Tensor, TArray<uint8> Buffer)
{
	return Tensor->CopyToBuffer(Buffer);
}
