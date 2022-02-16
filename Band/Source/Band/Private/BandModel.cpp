#include "BandModel.h"
#include "Band.h"
#include "BandLibraryWrapper.h"

const int32 UBandModel::GetModelHandle() const
{
    return ModelHandle;
}

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{   
    UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
    Band::TfLiteModel* ModelPtr = Band::TfLiteModelCreate(Buffer, Size);
    Model->ModelHandle = FBandModule::Get().RegisterModel(ModelPtr);
    UE_LOG(LogTemp, Log, TEXT("Band model registered %d"), Model->ModelHandle);
    Band::TfLiteModelDelete(ModelPtr);
    return Model->ModelHandle != -1 ? Model : nullptr;
}
