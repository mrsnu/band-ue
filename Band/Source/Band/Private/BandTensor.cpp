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

EBandStatus UBandTensor::CopyFromTexture(UTexture2D* Texture, int32 Mean, int32 Std)
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

	if ((PreviousSRGB != false) || (PreviousCompressionSettings != TC_VectorDisplacementmap))
	{
		ChangedTexture2D = true;
		Texture->SRGB = false;
		Texture->CompressionSettings = TC_VectorDisplacementmap;
		Texture->UpdateResource();
	}



	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	const size_t TypeBytes = BandEnum::TensorTypeBytes(Type());
	const int32 SizeX = Mip.SizeX;
	const int32 SizeY = Mip.SizeY;

	if (&Mip.BulkData == nullptr)
	{
		CleanUp();
		UE_LOG(LogBand, Log, TEXT("Texture Mip0 has nullptr"));
		return EBandStatus::Error;
	}
	
	const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
	const size_t NumTextureElements = SizeX * SizeY;

	UE_LOG(LogBand, Log, TEXT("CopyFromTexture: NumTextureElements: %d (%d * %d)"), NumTextureElements, SizeX, SizeY);

	bool Processed = true;
	if (NumTensorElements == NumTextureElements)
	{

		EPixelFormat PixelFormat = PF_B8G8R8A8;
#if !PLATFORM_LITTLE_ENDIAN
		PixelFormat = PF_R8G8B8A8;
#endif
		const uint8* SourceData = static_cast<const uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
		
		switch (Type())
		{
		case EBandTensorType::Float32:
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: float32"));
			BandTensorUtil::TextureToRGBArray<float>(SourceData, PixelFormat, reinterpret_cast<float*>(Data()), NumTensorElements, Mean, Std);
			break;
		case EBandTensorType::UInt8:
			BandTensorUtil::TextureToRGBArray<uint8>(SourceData, PixelFormat, Data(), NumTensorElements, Mean, Std);
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: UInt8"));
			break;
		case EBandTensorType::Int8:
			BandTensorUtil::TextureToRGBArray<int8>(SourceData, PixelFormat, reinterpret_cast<int8_t*>(Data()), NumTensorElements, Mean, Std);
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: Int8"));
			break;
		default:
			UE_LOG(LogBand, Log, TEXT("Texture elements %d != tensor elements %d"), NumTextureElements, NumTensorElements);
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
