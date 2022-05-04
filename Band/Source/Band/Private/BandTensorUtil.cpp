#include "BandTensorUtil.h"

TArray<TfLiteTensor*> BandTensorUtil::TensorsFromTArray(TArray<UBandTensor*> Tensors)
{
	TArray<TfLiteTensor*> OutTensors;
	
	for (int i = 0; i < Tensors.Num(); i++)
	{
		if (Tensors[i])
		{
			OutTensors.Push(Tensors[i]->Handle());
		}
		else
		{
			UE_LOG(LogBand, Error, TEXT("Try to access null tensor."));
		}
	}

	return OutTensors;
}