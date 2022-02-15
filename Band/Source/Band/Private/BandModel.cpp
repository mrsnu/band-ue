#include "BandModel.h"

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{   
    UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
    Model->ModelHandle = Band::TfLiteModelCreate(Buffer, Size);
    return Model->ModelHandle != nullptr ? Model : nullptr;
}
