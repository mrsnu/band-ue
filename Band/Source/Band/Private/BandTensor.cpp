#include "BandTensor.h"
#include "BandLibraryWrapper.h"

void UBandTensor::BeginDestroy()
{
	if (TensorHandle) {
		Band::TfLiteTensorDeallocate(TensorHandle);
		TensorHandle = nullptr;
	}
}

TfLiteType UBandTensor::Type()
{
	return Band::TfLiteTensorType(TensorHandle);
}

int32 UBandTensor::NumDims()
{
	return Band::TfLiteTensorNumDims(TensorHandle);
}

uint64 UBandTensor::ByteSize()
{
	return Band::TfLiteTensorByteSize(TensorHandle);
}

void* UBandTensor::Data()
{
	return Band::TfLiteTensorData(TensorHandle);
}

FString UBandTensor::Name()
{
	return FString(Band::TfLiteTensorName(TensorHandle));
}

TfLiteStatus UBandTensor::CopyFromBuffer(const TArray<uint8>& Buffer)
{
	return Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize());
}

TfLiteStatus UBandTensor::CopyToBuffer(TArray<uint8>& Buffer)
{
	return Band::TfLiteTensorCopyToBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize());
}
