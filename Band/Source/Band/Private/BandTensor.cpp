#include "BandTensor.h"
#include "BandTensorUtil.h"
#include "BandLibraryWrapper.h"
#include "Rendering/Texture2DResource.h"

#include "ImageUtil/FrameBuffer.h"
#include "ImageUtil/FrameBufferUtils.h"
#include "ImageUtil/FrameBufferCommonUtils.h"

void UBandTensor::BeginDestroy()
{
	if (TensorHandle)
	{
		Band::TfLiteTensorDeallocate(TensorHandle);
		TensorHandle = nullptr;
	}

	if (RGBBuffer)
	{
		delete[] RGBBuffer;
	}

	Super::BeginDestroy();
}

void UBandTensor::FromCameraFrame(const UAndroidCameraFrame *Frame, bool normalize)
{
	const UAndroidCameraFrame::NV12Frame &FrameData = Frame->GetData();
	std::unique_ptr<Band::FrameBufferUtils> Utils = Band::FrameBufferUtils::Create(Band::FrameBufferUtils::ProcessEngine::kLibyuv);
	std::unique_ptr<Band::FrameBuffer> YuvBuffer = Band::CreateFromYuvRawBuffer(
		FrameData.Y, FrameData.U, FrameData.V, Band::FrameBuffer::Format::kNV12,
		{Frame->GetWidth(), Frame->GetHeight()},
		FrameData.YRowStride, FrameData.UVRowStride, FrameData.UVPixelStride);

	// BWHC format
	const int InputWidth = Dim(1);
	const int InputHeight = Dim(2);

	// Directly update uint8 buffer
	uint8 *TargetBufferPtr = Type() == EBandTensorType::UInt8 ? Data() : RGBBuffer;
	std::unique_ptr<Band::FrameBuffer> OutputBuffer = Band::CreateFromRgbRawBuffer(TargetBufferPtr, {InputWidth, InputHeight});

	// Image preprocessing
	if (!Utils->Preprocess(*YuvBuffer, OutputBuffer.get()))
	{
		UE_LOG(LogBand, Display, TEXT("FromCameraFrame - Failed to preprocess"));
		return;
	}

	// Type conversion (RGB8 to tensor type)
	if (Type() != EBandTensorType::UInt8)
	{
		float Mean = normalize ? 127.5f : 0.f;
		float Std = normalize ? 127.5f : 1.f;
		const UEnum *EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
		switch (Type())
		{
		case EBandTensorType::Float32:
			BandTensorUtil::RGB8ToRGBArray<float>(TargetBufferPtr, reinterpret_cast<float *>(Data()), InputWidth * InputHeight, Mean, Std);
			break;
		default:
			UE_LOG(LogBand, Display, TEXT("FromCameraFrame - Failed to convert from %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
			break;
		}
	}
}

void UBandTensor::Initialize(TfLiteTensor *NewTensorHandle)
{
	TensorHandle = NewTensorHandle;

	RGBBuffer = new uint8[NumElements()];
}

EBandTensorType UBandTensor::Type()
{
	return EBandTensorType(Band::TfLiteTensorType(TensorHandle));
}

int32 UBandTensor::Dim(int32 Index)
{
	return Band::TfLiteTensorDim(TensorHandle, Index);
}

int32 UBandTensor::NumDims()
{
	return Band::TfLiteTensorNumDims(TensorHandle);
}

int32 UBandTensor::NumElements()
{
	int32 NumElements = 1;
	for (int i = 0; i < NumDims(); i++)
	{
		NumElements *= Dim(i);
	}
	return NumElements;
}

int32 UBandTensor::ByteSize()
{
	return (int32)Band::TfLiteTensorByteSize(TensorHandle);
}

uint8 *UBandTensor::Data()
{
	return (uint8 *)Band::TfLiteTensorData(TensorHandle);
}

FString UBandTensor::Name()
{
	return FString(Band::TfLiteTensorName(TensorHandle));
}

EBandStatus UBandTensor::CopyFromBuffer(uint8 *Buffer, int32 Bytes)
{
	if (Bytes != ByteSize())
	{
		UE_LOG(LogBand, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Bytes, ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer, Bytes));
}

EBandStatus UBandTensor::CopyFromBuffer(TArray<uint8> Buffer)
{
	if (ByteSize() != Buffer.GetAllocatedSize())
	{
		UE_LOG(LogBand, Log, TEXT("Buffer bytes %d != target tensor bytes %d"), Buffer.GetAllocatedSize(), ByteSize());
		return EBandStatus::Error;
	}
	return EBandStatus(Band::TfLiteTensorCopyFromBuffer(TensorHandle, Buffer.GetData(), Buffer.GetAllocatedSize()));
}

EBandStatus UBandTensor::CopyFromTexture(UTexture2D* Texture, float Mean, float Std)
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

	FTexture2DMipMap &Mip = Texture->PlatformData->Mips[0];
	if (&Mip.BulkData == nullptr)
	{
		CleanUp();
		UE_LOG(LogBand, Log, TEXT("Texture Mip0 has nullptr"));
		return EBandStatus::Error;
	}
	
	const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
	const size_t NumTextureElements = SizeX * SizeY;
	UE_LOG(LogBand, Log, TEXT("CopyFromTexture: NumTextureElements: %d (%d * %d)"), NumTextureElements, SizeX, SizeY);
	
	EPixelFormat TargetPixelFormat = Texture->PlatformData->PixelFormat;

	bool Processed = true;
	if (NumTensorElements == NumTextureElements)
	{
		const uint8* SourceData = static_cast<const uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
		
		switch (TensorType)
		{
		case EBandTensorType::Float32:
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
			BandTensorUtil::TextureToRGBArray<float>(SourceData, TargetPixelFormat, reinterpret_cast<float*>(Data()), NumTensorElements, Mean, Std);
			break;
		case EBandTensorType::UInt8:
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
			BandTensorUtil::TextureToRGBArray<uint8>(SourceData, TargetPixelFormat, Data(), NumTensorElements, Mean, Std);
			break;
		case EBandTensorType::Int8:
			UE_LOG(LogBand, Log, TEXT("CopyFromTexture - EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
			BandTensorUtil::TextureToRGBArray<int8>(SourceData, TargetPixelFormat, reinterpret_cast<int8_t*>(Data()), NumTensorElements, Mean, Std);
			break;
		default:
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
