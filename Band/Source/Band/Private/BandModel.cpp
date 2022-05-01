#include "BandModel.h"
#include "Band.h"
#include "BandLibraryWrapper.h"

bool UBandModel::IsRegistered() const
{
	return Registered;
}

const int32 UBandModel::GetHandle()
{
	// Delay-register model
	// TODO(dostos): figure out how to register
	// when UObject imported to editor as an uasset
	if (!IsRegistered())
	{
		RegisterModel();
	}
	return ModelHandle;
}

const TArray<uint8>& UBandModel::GetBinary() const
{
	return ModelBinary;
}

void UBandModel::RegisterModel()
{
	std::unique_lock<std::mutex> RegisterLock(RegisterMutex);
	ModelHandle = FBandModule::Get().RegisterModel(this);
	if (ModelHandle != -1)
	{
		Registered = true;
	}
}

UBandTensor* UBandModel::AllocateInputTensor(int32 InputIndex)
{
	return FBandModule::Get().AllocateInputTensor(this, InputIndex);
}

UBandTensor* UBandModel::AllocateOutputTensor(int32 OutputIndex)
{
	return FBandModule::Get().AllocateOutputTensor(this, OutputIndex);
}

int32 UBandModel::GetInputTensorCount()
{
	return FBandModule::Get().GetInputTensorCount(this);
}

int32 UBandModel::GetOutputTensorCount()
{
	return FBandModule::Get().GetOutputTensorCount(this);
}

void UBandModel::InvokeSync(UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	if (!Registered)
	{
		UE_LOG(LogBand, Error, TEXT("%s: Model not registered"), ANSI_TO_TCHAR(__FUNCTION__));
	}
	FBandModule::Get().InvokeSync(this, InputTensors, OutputTensors);
}

int32 UBandModel::InvokeAsync(UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	if (!Registered)
	{
		UE_LOG(LogBand, Error, TEXT("%s: Model not registered"), ANSI_TO_TCHAR(__FUNCTION__));
	}
	return FBandModule::Get().InvokeAsync(this, InputTensors);
}

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{
	UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
	Model->ModelBinary = TArray<uint8>(Buffer, Size);
	Model->RegisterModel();
	return Model;
}
