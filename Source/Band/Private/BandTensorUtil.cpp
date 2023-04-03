#include "BandTensorUtil.h"

TArray<BandTensor*> BandTensorUtil::TensorsFromTArray(
    TArray<UBandTensor*> Tensors) {
  TArray<BandTensor*> OutTensors;

  for (int i = 0; i < Tensors.Num(); i++) {
    if (Tensors[i]) {
      OutTensors.Push(Tensors[i]->Handle());
    } else {
      UE_LOG(LogBand, Error, TEXT("Try to access null tensor."));
    }
  }

  return OutTensors;
}
