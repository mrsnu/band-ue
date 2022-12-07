#include "BandTensor.h"
#include "Band.h"
#include "BandTensorUtil.h"
#include "BandLibrary.h"
#include "Rendering/Texture2DResource.h"

#include "ImageUtil/FrameBuffer.h"
#include "ImageUtil/FrameBufferUtils.h"
#include "ImageUtil/FrameBufferCommonUtils.h"

void UBandTensor::BeginDestroy()
{
	if (TensorHandle)
	{
		FBandModule::Get().BandTensorDelete(TensorHandle);
		TensorHandle = nullptr;
	}

	if (RGBBuffer)
	{
		delete[] RGBBuffer;
	}

	Super::BeginDestroy();
}

void UBandTensor::FromCameraFrame(UPARAM(ref) const UAndroidCameraFrame* Frame, bool Normalize)
{
	if (!Frame)
	{
		UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Something went wrong, Null frame"));
		return;
	}
	if (!TensorHandle)
	{
		UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Something went wrong, Null tensor"));
		return;
	}
	SCOPE_CYCLE_COUNTER(STAT_BandCameraToTensor);
	
	std::unique_ptr<Band::FrameBuffer> Buffer;
	std::unique_ptr<Band::FrameBufferUtils> Utils = Band::FrameBufferUtils::Create(Band::FrameBufferUtils::ProcessEngine::kLibyuv);;
	if(Frame->HasYUV()){
		const UAndroidCameraFrame::NV12Frame& FrameData = Frame->GetData();
		Buffer = Band::CreateFromYuvRawBuffer(
			FrameData.Y, FrameData.U, FrameData.V, Band::FrameBuffer::Format::kNV12,
			{ Frame->GetWidth(), Frame->GetHeight() },
			FrameData.YRowStride, FrameData.UVRowStride, FrameData.UVPixelStride);
	}
	else if (Frame->GetARGBBuffer())
	{
		const uint8* FrameData = Frame->GetARGBBuffer();
		Buffer = Band::CreateFromRgbaRawBuffer(FrameData, {Frame->GetWidth(), Frame->GetHeight()});
	}
	
	const float Mean = Normalize ? 127.5f : 0.f;
	const float Std = Normalize ? 127.5f : 1.f;

	if (Buffer.get())
	{
		// BWHC format
		const int InputWidth = Dim(1);
		const int InputHeight = Dim(2);

		// Directly update uint8 buffer
		uint8* TargetBufferPtr = Type() == EBandTensorType::UInt8 ? Data() : RGBBuffer;
		std::unique_ptr<Band::FrameBuffer> OutputBuffer = Band::CreateFromRgbRawBuffer(TargetBufferPtr, { InputWidth, InputHeight });
		// Image preprocessing
		if (!Utils->Preprocess(*Buffer, OutputBuffer.get()))
		{
			UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Failed to preprocess"));
			return;
		}

		// Type conversion (RGB8 to tensor type)
		if (Type() != EBandTensorType::UInt8)
		{
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			switch (Type())
			{
				case EBandTensorType::Float32:
					BandTensorUtil::RGB8ToRGBArray<float>(TargetBufferPtr, reinterpret_cast<float*>(Data()), InputWidth * InputHeight, Mean, Std);
				break;
				default:
					UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Failed to convert from %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
				break;
			}
		}
	}
	else if (Frame->GetTexture2D())
	{
		CopyFromTexture(Frame->GetTexture2D(), Mean, Std);
	}
	else
	{
		UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Failed to copy from both buffer (YUV or ARGB) and texture"));
	}
}

template <typename T>
std::pair<size_t, T> TemplatedArgMax(T* Buffer, size_t Length)
{
	std::pair<size_t, T> BestIndexValue = { 0, std::numeric_limits<T>::min() };
	for (size_t i = 0; i < Length; i++)
	{
		if (Buffer[i] > BestIndexValue.second)
		{
			BestIndexValue = { i, Buffer[i] };
		}
	}
	return BestIndexValue;
}

void UBandTensor::ArgMax(int32& Index, float& Value)
{
	switch (Type())
	{
		case EBandTensorType::Float32:
			std::tie(Index, Value) = TemplatedArgMax<float>(reinterpret_cast<float*>(Data()), NumElements());
			break;
		case EBandTensorType::Int32:
			std::tie(Index, Value) = TemplatedArgMax<int32>(reinterpret_cast<int32*>(Data()), NumElements());
			break;
		case EBandTensorType::UInt8:
			std::tie(Index, Value) = TemplatedArgMax<uint8>(reinterpret_cast<uint8*>(Data()), NumElements());
			break;
		case EBandTensorType::Int64:
			std::tie(Index, Value) = TemplatedArgMax<int64>(reinterpret_cast<int64*>(Data()), NumElements());
			break;
		case EBandTensorType::Int16:
			std::tie(Index, Value) = TemplatedArgMax<int16>(reinterpret_cast<int16*>(Data()), NumElements());
			break;
		case EBandTensorType::Complex64:
			std::tie(Index, Value) = TemplatedArgMax<double>(reinterpret_cast<double*>(Data()), NumElements());
			break;
		case EBandTensorType::Int8:
			std::tie(Index, Value) = TemplatedArgMax<int8>(reinterpret_cast<int8*>(Data()), NumElements());
			break;
		case EBandTensorType::Float64:
			std::tie(Index, Value) = TemplatedArgMax<double>(reinterpret_cast<double*>(Data()), NumElements());
			break;
		default:
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			UE_LOG(LogBand, Error, TEXT("ArgMax: Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
			break;
	}
}

void UBandTensor::Initialize(BandTensor* NewTensorHandle)
{
	TensorHandle = NewTensorHandle;
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
	FString DimensionString;
	for (int32 i = 0; i < NumDims(); i++)
	{
		DimensionString += FString::FromInt(Dim(i));
		if (i < NumDims() - 1)
		{
			DimensionString += "x";
		}
	}
	UE_LOG(LogBand, Log, TEXT("Allocate new Tensor dimension(%s) name(%s) type(%s)"),
		*DimensionString,
		*Name(),
		*EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));

	RGBBuffer = new uint8[NumElements()];
}

EBandTensorType UBandTensor::Type()
{
	return EBandTensorType(FBandModule::Get().BandTensorGetType(TensorHandle));
}

int32 UBandTensor::Dim(int32 Index)
{
	if (Index < NumDims() && Index >= 0)
	{
		return FBandModule::Get().BandTensorGetDims(TensorHandle)[Index];
	} else
	{
		UE_LOG(LogBand, Error, TEXT("Dim: out of index %d"), Index);
		return -1;
	}
}

int32 UBandTensor::NumDims()
{
	return FBandModule::Get().BandTensorGetNumDims(TensorHandle);
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
	return (int32)FBandModule::Get().BandTensorGetBytes(TensorHandle);
}

uint8* UBandTensor::Data()
{
	return (uint8*)FBandModule::Get().BandTensorGetData(TensorHandle);
}

FString UBandTensor::Name()
{
	return FString(FBandModule::Get().BandTensorGetName(TensorHandle));
}

TArray<uint8> UBandTensor::GetUInt8Buffer()
{
	return { Data(), ByteSize() };
}

TArray<float> UBandTensor::GetF32Buffer()
{
	return TArray<float>((float*)(Data()), ByteSize() / sizeof(float));
}

EBandStatus UBandTensor::CopyFromBuffer(uint8* Buffer, int32 Bytes)
{
	if (Bytes != ByteSize())
	{
		UE_LOG(LogBand, Error, TEXT("CopyFromBuffer: Buffer bytes %d != target tensor bytes %d"), Bytes, ByteSize());
		return EBandStatus::Error;
	}
	memcpy(Data(), Buffer, Bytes);
	return EBandStatus::Ok;
}

EBandStatus UBandTensor::CopyFromBuffer(TArray<uint8> Buffer)
{
	if (ByteSize() != Buffer.GetAllocatedSize())
	{
		UE_LOG(LogBand, Error, TEXT("CopyFromBuffer: Buffer bytes %llu != target tensor bytes %d"), Buffer.GetAllocatedSize(), ByteSize());
		return EBandStatus::Error;
	}
	memcpy(Data(), Buffer.GetData(), Buffer.GetAllocatedSize());
	return EBandStatus::Ok;
}

EBandStatus UBandTensor::CopyFromTexture(UPARAM(ref) UTexture2D* Texture, float Mean, float Std)
{
	// TODO(@juimdpp) Resize texture to allow any texture to be processed
	SCOPE_CYCLE_COUNTER(STAT_BandTextureToTensor);
	if (!Texture->PlatformData->Mips.Num())
	{
		UE_LOG(LogBand, Error, TEXT("CopyFromTexture: No available mips from texture"));
		return EBandStatus::Error;
	}

	bool ChangedTexture2D = false;
	bool PreviousSRGB = Texture->SRGB;
	TextureCompressionSettings PreviousCompressionSettings = Texture->CompressionSettings;

	auto CleanUp = [&]() {
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

	const EBandTensorType TensorType = Type();
	const size_t TypeBytes = BandEnum::TensorTypeBytes(TensorType);
	const int32 SizeX = Texture->PlatformData->Mips[0].SizeX;
	const int32 SizeY = Texture->PlatformData->Mips[0].SizeY;

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
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
		if (SourceData)
		{
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);

			switch (TensorType)
			{
				case EBandTensorType::Float32:
					UE_LOG(LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
					BandTensorUtil::TextureToRGBArray<float>(SourceData, TargetPixelFormat, reinterpret_cast<float*>(Data()), NumTensorElements, Mean, Std);
					break;
				case EBandTensorType::UInt8:
					UE_LOG(LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
					BandTensorUtil::TextureToRGBArray<uint8>(SourceData, TargetPixelFormat, Data(), NumTensorElements, Mean, Std);
					break;
				case EBandTensorType::Int8:
					UE_LOG(LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
					BandTensorUtil::TextureToRGBArray<int8>(SourceData, TargetPixelFormat, reinterpret_cast<int8_t*>(Data()), NumTensorElements, Mean, Std);
					break;
				default:
					UE_LOG(LogBand, Error, TEXT("CopyFromTexture: Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
					Processed = false;
					break;
			}
		}
		else
		{
			UE_LOG(LogBand, Error, TEXT("CopyFromTexture: Tried to access null source data"));
		}

		Mip.BulkData.Unlock();
	}
	else
	{
		UE_LOG(LogBand, Error, TEXT("CopyFromTexture: Texture elements %llu != tensor elements %llu"), NumTextureElements, NumTensorElements);
		Processed = false;
	}

	CleanUp();
	return Processed ? EBandStatus::Ok : EBandStatus::Error;
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer)
{
	if (ByteSize() != Buffer.GetAllocatedSize())
	{
		UE_LOG(LogBand, Error, TEXT("CopyToBuffer: Buffer bytes %llu != target tensor bytes %d"), Buffer.GetAllocatedSize(), ByteSize());
		return EBandStatus::Error;
	}
	memcpy(Buffer.GetData(), Data(), Buffer.GetAllocatedSize());
	return EBandStatus::Ok;
}

BandTensor* UBandTensor::Handle() const
{
	return TensorHandle;
}
