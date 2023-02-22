#include "BandModel.h"
#include "Band.h"
#include "BandLibrary.h"

bool UBandModel::IsRegistered() const {
  std::unique_lock<std::mutex> RegisterLock(RegisterMutex);
  return ModelHandle != nullptr;
}

void UBandModel::BeginDestroy() {
  if (ModelHandle) {
    FBandModule::Get().BandModelDelete(ModelHandle);
  }
  Super::BeginDestroy();
}

BandModel* UBandModel::GetHandle() const {
  // Delay-register model
  // TODO(dostos): figure out how to register
  // when UObject imported to editor as an uasset
  if (!IsRegistered()) {
    RegisterModel();
  }
  return ModelHandle;
}

const TArray<uint8>& UBandModel::GetBinary() const {
  return ModelBinary;
}

void UBandModel::RegisterModel() const {
  std::unique_lock<std::mutex> RegisterLock(RegisterMutex);
  if (ModelBinary.Num() && ModelHandle == nullptr) {
    ModelHandle = FBandModule::Get().BandModelCreate();
    if (FBandModule::Get().BandModelAddFromBuffer(
            ModelHandle, kBandTfLite, ModelBinary.GetData(),
            ModelBinary.Num()) == kBandOk
        && FBandModule::Get().BandEngineRegisterModel(
            FBandModule::Get().GetEngineHandle(), ModelHandle) == kBandOk) {
    } else {
      FBandModule::Get().BandModelDelete(ModelHandle);
    }
  }
}

UBandModel* UBandModel::LoadModel(UObject* InParent, FName InName,
                                  EObjectFlags Flags, const uint8*& Buffer,
                                  size_t Size) {
  UBandModel* Model = NewObject<UBandModel>(InParent, InName, Flags);
  Model->ModelBinary = TArray<uint8>(Buffer, Size);
  Model->RegisterModel();
  return Model;
}
