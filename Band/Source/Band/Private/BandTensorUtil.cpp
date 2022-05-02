#include "BandTensorUtil.h"

TArray<TfLiteTensor*> BandTensorUtil::TensorsFromTArray(TArray<UBandTensor*> Tensors)
{
	TArray<TfLiteTensor*> OutTensors;

	for (int i = 0; i < Tensors.Num(); i++)
	{
		OutTensors.Push(Tensors[i]->Handle());
	}

	return OutTensors;
}