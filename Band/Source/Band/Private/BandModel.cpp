#include "BandModel.h"
#include "Band.h"
#include "BandLibraryWrapper.h"

#define ENSURE_REGISTERED                                     \
    if (!Registered)                                          \
    {                                                         \
        ReportUnregisteredError(ANSI_TO_TCHAR(__FUNCTION__)); \
        return {};                                            \
    }

bool UBandModel::IsRegistered() const
{
    return Registered;
}

void UBandModel::ReportUnregisteredError(const TCHAR *FunctionName) const
{
    UE_LOG(LogBand, Error, TEXT("%s: Model not registered"), FunctionName);
}

const int32 UBandModel::GetModelHandle()
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

void UBandModel::RegisterModel()
{
    if (ModelBinary.Num() > 0)
    {
        Band::TfLiteModel *TfLiteModel = Band::TfLiteModelCreate(ModelBinary.GetData(), ModelBinary.Num());
        ModelHandle = Band::TfLiteInterpreterRegisterModel(FBandModule::Get().GetInterpreter(), TfLiteModel);
        Band::TfLiteModelDelete(TfLiteModel);
        Registered = true;
    }
}

UBandTensor *UBandModel::AllocateInputTensor(int32 InputIndex)
{
    ENSURE_REGISTERED
    return FBandModule::Get().AllocateInputTensor(this, InputIndex);
}

UBandTensor *UBandModel::AllocateOutputTensor(int32 OutputIndex)
{
    ENSURE_REGISTERED
    return FBandModule::Get().AllocateOutputTensor(this, OutputIndex);
}

int32 UBandModel::GetInputTensorCount()
{
    ENSURE_REGISTERED
    return FBandModule::Get().GetInputTensorCount(this);
}

int32 UBandModel::GetOutputTensorCount()
{
    ENSURE_REGISTERED
    return FBandModule::Get().GetOutputTensorCount(this);
}

void UBandModel::InvokeSync(UPARAM(ref) TArray<UBandTensor *> InputTensors, UPARAM(ref) TArray<UBandTensor *> OutputTensors)
{
    if (Registered)
    {
        FBandModule::Get().InvokeSync(this, InputTensors, OutputTensors);
    }
    else
    {
        ReportUnregisteredError(ANSI_TO_TCHAR(__FUNCTION__));
    }
}

int32 UBandModel::InvokeAsync(UPARAM(ref) TArray<UBandTensor *> InputTensors, UPARAM(ref) TArray<UBandTensor *> OutputTensors)
{
    ENSURE_REGISTERED
    return FBandModule::Get().InvokeAsync(this, InputTensors);
}

UBandModel *UBandModel::LoadModel(UObject *InParent, FName InName, EObjectFlags Flags, const uint8 *&Buffer, size_t Size)
{
    UBandModel *Model = NewObject<UBandModel>(InParent, InName, Flags);
    Model->ModelBinary = TArray<uint8>(Buffer, Size);
    Model->RegisterModel();
    return Model;
}
