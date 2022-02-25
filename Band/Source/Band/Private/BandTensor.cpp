#include "BandTensor.h"
#include "BandTensorUtil.h"
#include "BandLibraryWrapper.h"

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

int32 UBandTensor::ByteSize()
{
	return (int32)Band::TfLiteTensorByteSize(TensorHandle);
}

uint8* UBandTensor::Data()
{
	return (uint8*)Band::TfLiteTensorData(TensorHandle);
}

FString UBandTensor::Name()
{
	return FString(Band::TfLiteTensorName(TensorHandle));
}

EBandStatus UBandTensor::CopyFromBuffer(uint8* Buffer, int32 Bytes)
{
	if (Bytes != ByteSize()) {
		UE_LOG(LogBand, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Bytes, ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer, Bytes));
}

EBandStatus UBandTensor::CopyFromBuffer(TArray<uint8> Buffer)
{
	if (ByteSize() != Buffer.GetAllocatedSize()) {
		UE_LOG(LogBand, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Buffer.GetAllocatedSize(), ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}

EBandStatus UBandTensor::CopyFromTexture(UTexture* Texture)
{
	FTextureSource& Source = Texture->Source;
	const size_t TypeBytes = BandEnum::TensorTypeBytes(Type());
	const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
	const size_t NumTextureElements = Source.GetSizeX() * Source.GetSizeY();
	if (NumTensorElements != NumTextureElements)
	{
		switch (Type())
		{
		case EBandTensorType::Float32:
			BandTensorUtil::TextureToRGBArray<float>(Source, (float*)Data(), 127.5f, 127.5f);
			break;
		case EBandTensorType::UInt8:
			BandTensorUtil::TextureToRGBArray<uint8>(Source, Data(), 0, 255);
			break;
		case EBandTensorType::Int8:
			BandTensorUtil::TextureToRGBArray<int8>(Source, (int8*)Data(), 0, 127);
			break;
		default:
			UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
			return EBandStatus::Error;
			break;
		}
	}
	else
	{
		UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
		return EBandStatus::Error;
	}

	return EBandStatus::Ok;
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer)
{
	return EBandStatus(Band::TfLiteTensorCopyToBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}
