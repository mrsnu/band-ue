#include "BandTensor.h"
#include "BandLibraryWrapper.h"
#include "..\Public\BandTensor.h"

void UBandTensor::BeginDestroy()
{
	if (TensorHandle) {
		Band::TfLiteTensorDeallocate(TensorHandle);
		TensorHandle = nullptr;
	}
	Super::BeginDestroy();
}

EBandTensorType UBandTensor::Type()
{
	return EBandTensorType(Band::TfLiteTensorType(TensorHandle));
}

int32 UBandTensor::NumDims()
{
	return Band::TfLiteTensorNumDims(TensorHandle);
}

uint64 UBandTensor::ByteSize()
{
	return Band::TfLiteTensorByteSize(TensorHandle);
}

TArray<uint8> UBandTensor::Data()
{
	return TArray<uint8>((uint8*)Band::TfLiteTensorData(TensorHandle), ByteSize());
}

FString UBandTensor::Name()
{
	return FString(Band::TfLiteTensorName(TensorHandle));
}

EBandStatus UBandTensor::CopyFromBuffer(uint8* Buffer, int32 Bytes)
{
	if (Bytes != ByteSize()) {
		UE_LOG(LogTemp, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Bytes, ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer, Bytes));
}

EBandStatus UBandTensor::CopyFromBuffer(TArray<uint8> Buffer)
{
	if (ByteSize() != Buffer.GetAllocatedSize()) {
		UE_LOG(LogTemp, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Buffer.GetAllocatedSize(), ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer)
{
	return EBandStatus(Band::TfLiteTensorCopyToBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}
