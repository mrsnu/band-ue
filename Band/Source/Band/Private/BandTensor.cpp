#include "BandTensor.h"
#include "BandTensorUtil.h"
#include "BandLibraryWrapper.h"
#include "Rendering/Texture2DResource.h"

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

EBandStatus UBandTensor::CopyFromTexture(UTexture2D* Texture)
{
	if (!Texture->PlatformData->Mips.Num())
	{
		UE_LOG(LogBand, Log, TEXT("No available mips from texture"));
		return EBandStatus::Error;
	}

	FTexture2DResource* Resource = (FTexture2DResource*)Texture->Resource;
	
	const size_t TypeBytes = BandEnum::TensorTypeBytes(Type());
	const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
	const size_t NumTextureElements = Resource->GetSizeX() * Resource->GetSizeY();
	bool Processed = true;
	if (NumTensorElements != NumTextureElements)
	{
		uint32 Stride = 0; // Assigned by RHILockTexture2D.
		const FColor* Source = static_cast<const FColor*>(RHILockTexture2D(
			Resource->GetTexture2DRHI(),
			0,
			RLM_ReadOnly,
			Stride,
			false));
		EBandTensorType TensorType = Type();
		EPixelFormat PixelFormat = Resource->GetPixelFormat();
		switch (TensorType)
		{
		case EBandTensorType::Float32:
			BandTensorUtil::TextureToRGBArray<float>(Source, PixelFormat, reinterpret_cast<float*>(Data()), NumTensorElements, 127.5f, 127.5f);
			break;
		case EBandTensorType::UInt8:
			BandTensorUtil::TextureToRGBArray<uint8>(Source, PixelFormat, Data(), NumTensorElements, 0, 255);
			break;
		case EBandTensorType::Int8:
			BandTensorUtil::TextureToRGBArray<int8>(Source, PixelFormat, reinterpret_cast<int8_t*>(Data()), NumTensorElements, 0, 127);
			break;
		default:
			UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
			Processed = false;
			break;
		}
		RHIUnlockTexture2D(Resource->GetTexture2DRHI(), 0, false);
	}
	else
	{
		UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
		Processed = false;
	}

	return Processed ? EBandStatus::Ok : EBandStatus::Error;
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer)
{
	return EBandStatus(Band::TfLiteTensorCopyToBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}
