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
	if (ModelBinary.Num() && !IsRegistered())
	{
		Band::TfLiteModel* TfLiteModel = FBandModule::Get().TfLiteModelCreate(ModelBinary.GetData(), ModelBinary.Num());
		ModelHandle = FBandModule::Get().TfLiteInterpreterRegisterModel(FBandModule::Get().GetInterpreterHandle() , TfLiteModel);
		FBandModule::Get().TfLiteModelDelete(TfLiteModel);
		if (ModelHandle != -1)
		{
			Registered = true;
		}
	}
}

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{
	UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
	Model->ModelBinary = TArray<uint8>(Buffer, Size);
	Model->RegisterModel();
	return Model;
}
