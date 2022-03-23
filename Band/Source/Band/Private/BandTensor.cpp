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

void UBandTensor::FromCameraFrame(const UAndroidCameraFrame* Frame)
{
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


	bool ChangedTexture2D = false;
	bool PreviousSRGB = Texture->SRGB;
	TextureCompressionSettings PreviousCompressionSettings = Texture->CompressionSettings;

	auto CleanUp = [&]()
	{
		if (ChangedTexture2D)
		{
			Texture->SRGB = PreviousSRGB;
			Texture->CompressionSettings = PreviousCompressionSettings;
			Texture->UpdateResource();
		}
	};
	UE_LOG(LogBand, Log, TEXT("Texture pixel format before conversion %d"), Texture->PlatformData->PixelFormat);

	if ((PreviousSRGB != false) || (PreviousCompressionSettings != TC_VectorDisplacementmap))
	{
		ChangedTexture2D = true;
		Texture->SRGB = false;
		Texture->CompressionSettings = TC_VectorDisplacementmap;
		Texture->UpdateResource();
	}

	const EBandTensorType TensorType = Type();
	const size_t TypeBytes = BandEnum::TensorTypeBytes(TensorType);
	const int32 SizeX = Texture->PlatformData->Mips[0].SizeX;
	const int32 SizeY = Texture->PlatformData->Mips[0].SizeY;
	UE_LOG(LogBand, Log, TEXT("Texture pixel format after conversion %d"), Texture->PlatformData->PixelFormat);

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	if (&Mip.BulkData == nullptr)
	{
		CleanUp();
		UE_LOG(LogBand, Log, TEXT("Texture Mip0 has nullptr"));
		return EBandStatus::Error;
	}

	const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
	const size_t NumTextureElements = SizeX * SizeY;

	EPixelFormat TargetPixelFormat = Texture->PlatformData->PixelFormat;

	bool Processed = true;
	if (NumTensorElements == NumTextureElements)
	{
		const uint8* SourceData = static_cast<const uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
		
		switch (TensorType)
		{
		case EBandTensorType::Float32:
			BandTensorUtil::TextureToRGBArray<float>(SourceData, TargetPixelFormat, reinterpret_cast<float*>(Data()), NumTensorElements, 127.5f, 127.5f);
			break;
		case EBandTensorType::UInt8:
			BandTensorUtil::TextureToRGBArray<uint8>(SourceData, TargetPixelFormat, Data(), NumTensorElements, 0, 1);
			break;
		case EBandTensorType::Int8:
			BandTensorUtil::TextureToRGBArray<int8>(SourceData, TargetPixelFormat, reinterpret_cast<int8_t*>(Data()), NumTensorElements, 0, 1);
			break;
		default:
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			UE_LOG(LogBand, Log, TEXT("Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
			Processed = false;
			break;
		}

		Mip.BulkData.Unlock();
	}
	else
	{
		UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
		Processed = false;
	}

	CleanUp();
	return Processed ? EBandStatus::Ok : EBandStatus::Error;
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer)
{
	return EBandStatus(Band::TfLiteTensorCopyToBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}
