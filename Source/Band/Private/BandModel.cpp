#include "BandModel.h"
#include "Band.h"
#include "BandTensor.h"
#include "libband.h"


void UBandModel::BeginDestroy() {
  if (Handle) {
    FBandModule::Get().DeleteModel(this);
  }
  Super::BeginDestroy();
}

int32 UBandModel::GetInputTensorCount() const {
  return FBandModule::Get().GetInputTensorCount(this);
}

int32 UBandModel::GetOutputTensorCount() const {
  return FBandModule::Get().GetOutputTensorCount(this);
}

UBandTensor *UBandModel::AllocateInputTensor(int32 InputIndex) const {
  return FBandModule::Get().CreateInputTensor(this, InputIndex);
}

UBandTensor *UBandModel::AllocateOutputTensor(int32 OutputIndex) const {
  return FBandModule::Get().CreateOutputTensor(this, OutputIndex);
}

TArray<UBandTensor *> UBandModel::AllocateInputTensors() const {
  TArray<UBandTensor *> Tensors;
  int32 InputTensorCount = GetInputTensorCount();
  for (int32 i = 0; i < InputTensorCount; ++i) {
    Tensors.Add(AllocateInputTensor(i));
  }
  return Tensors;
}

TArray<UBandTensor *> UBandModel::AllocateOutputTensors() const {
  TArray<UBandTensor *> Tensors;
  int32 OutputTensorCount = GetOutputTensorCount();
  for (int32 i = 0; i < OutputTensorCount; ++i) {
    Tensors.Add(AllocateOutputTensor(i));
  }
  return Tensors;
}

BandModel *UBandModel::GetHandle() const {
  if (Handle == nullptr) {
    FBandModule::Get().RegisterModel(this);
  }
  return Handle;
}

UBandModel *UBandModel::LoadModel(UObject *InParent, FName InName,
                                  EObjectFlags Flags, const uint8 *&Buffer,
                                  size_t Size) {
  UBandModel *Model = NewObject<UBandModel>(InParent, InName, Flags);
  Model->ModelBinary = TArray<uint8>(Buffer, Size);
  FBandModule::Get().RegisterModel(Model);
  return Model;
}

const TArray<uint8> &UBandModel::GetBinary() const { return ModelBinary; }
