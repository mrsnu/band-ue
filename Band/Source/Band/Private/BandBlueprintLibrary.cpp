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

EBandTensorType UBandBlueprintLibrary::GetTensorType(UPARAM(ref) UBandTensor* Tensor)
{
	return Tensor->Type();
}

int32 UBandBlueprintLibrary::NumDims(UPARAM(ref) UBandTensor* Tensor)
{
	return Tensor->NumDims();
}

int32 UBandBlueprintLibrary::ByteSize(UPARAM(ref) UBandTensor* Tensor)
{
	return Tensor->ByteSize();
}

TArray<uint8> UBandBlueprintLibrary::GetRawBuffer(UPARAM(ref) UBandTensor* Tensor)
{
	TArray<uint8> Buffer = Tensor->Data();
	UE_LOG(LogTemp, Log, TEXT("%s"), *BytesToString(Buffer.GetData(), Buffer.Num()));
	return Tensor->Data();
}

TArray<float> UBandBlueprintLibrary::GetF32Buffer(UPARAM(ref)UBandTensor* Tensor)
{
	return TArray<float>((float*)(Tensor->Data().GetData()), Tensor->ByteSize() / sizeof(float));
}

FString UBandBlueprintLibrary::GetName(UPARAM(ref) UBandTensor* Tensor)
{
	return Tensor->Name();
}

EBandStatus UBandBlueprintLibrary::CopyFromBuffer(UPARAM(ref) UBandTensor* Tensor, TArray<uint8> Buffer)
{
	return Tensor->CopyFromBuffer(Buffer);
}

EBandStatus UBandBlueprintLibrary::CopyFromTexture(UPARAM(ref) UBandTensor* Tensor, UTexture* Texture)
{
	FTextureSource& Source = Texture->Source;
	if (Source.GetFormat() == TSF_BGRA8) {
		uint8* OffsetSource = Source.LockMip(0);
		Tensor->CopyFromBuffer(OffsetSource, Tensor->ByteSize());
		Source.UnlockMip(0);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Unknown source format %d"), Source.GetFormat());
		return EBandStatus::Error;
	}

	return EBandStatus::Ok;
}

EBandStatus UBandBlueprintLibrary::CopyToBuffer(UPARAM(ref) UBandTensor* Tensor, TArray<uint8> Buffer)
{
	return Tensor->CopyToBuffer(Buffer);
}
