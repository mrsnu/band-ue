#include "BandModel.h"
#include "Band.h"
#include "BandLibraryWrapper.h"

bool UBandModel::IsRegistered() const
{
    return Registered;
}
const int32 UBandModel::GetModelHandle()
{
    // Delay-register model 
    // TODO(dostos): figure out how to register
    // when UObject imported to editor as an uasset
    if (!IsRegistered()) {
        RegisterModel();
    }
    return ModelHandle;
}

void UBandModel::RegisterModel() {
    if (ModelBinary.Num() > 0) {
        Band::TfLiteModel* TfLiteModel = Band::TfLiteModelCreate(ModelBinary.GetData(), ModelBinary.Num());
        ModelHandle = Band::TfLiteInterpreterRegisterModel(FBandModule::Get().GetInterpreter(), TfLiteModel);
        Band::TfLiteModelDelete(TfLiteModel);
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
    FBandModule::Get().InvokeSync(this, InputTensors, OutputTensors);
}

int32 UBandModel::InvokeAsync(UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
    return FBandModule::Get().InvokeAsync(this, InputTensors);
}

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{
    UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
    Model->ModelBinary = TArray<uint8>(Buffer, Size);
    Model->RegisterModel();
    return Model;
}
