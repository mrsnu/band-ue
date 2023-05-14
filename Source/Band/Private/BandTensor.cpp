#include "BandTensor.h"

#include <fstream>

#include "Band.h"
#include "BandLibrary.h"
#include "BandTensorUtil.h"
#include "ImageUtil/FrameBufferCommonUtils.h"
#include "ImageUtil/FrameBufferUtils.h"
#include "Rendering/Texture2DResource.h"

void UBandTensor::BeginDestroy() {
  if (TensorHandle) {
    FBandModule::Get().BandTensorDelete(TensorHandle);
    TensorHandle = nullptr;
  }

  if (RGBBuffer) {
    delete[] RGBBuffer;
  }

  Super::BeginDestroy();
}

void UBandTensor::FromCameraFrame(const UAndroidCameraFrame* Frame,
                                  const float Mean, const float Std) {
  FromCameraFrameWithCrop(Frame, {}, Mean, Std);
}

void UBandTensor::FromCameraFrameWithCrop(UPARAM(ref)
    const UAndroidCameraFrame* Frame,
    FBandBoundingBox RoI,
    const float Mean,
    const float Std) {
  if (!Frame) {
    UE_LOG(LogBand, Display,
           TEXT("FromCameraFrame: Something went wrong, Null Frame"));
    return;
  }
  if (!TensorHandle) {
    UE_LOG(LogBand, Display,
           TEXT("FromCameraFrame: Something went wrong, Null tensor"));
    return;
  }
  SCOPE_CYCLE_COUNTER(STAT_BandCameraToTensor);

  std::unique_ptr<band::FrameBuffer> Buffer =
      band::CreateFromAndroidCameraFrame(*Frame);

  if (Buffer.get()) {
    CopyFromFrameBuffer(std::move(Buffer), RoI, Mean, Std);
  } else if (Frame->GetTexture2D()) {
    CopyFromTextureWithCrop(Frame->GetTexture2D(), RoI, Mean, Std);
  } else {
    UE_LOG(LogBand, Display,
           TEXT("FromCameraFrame: Failed to copy from both Buffer (YUV or "
             "ARGB) and Texture"));
  }
}

// TODO (juimdpp): this is rough implementation for Hand Gesture. Implement more
// general implementation
void UBandTensor::FromBoundingBox(UPARAM(ref) const FBandBoundingBox BBox) {
  if (!TensorHandle) {
    UE_LOG(LogBand, Display,
           TEXT("FromCameraFrame: Something went wrong, Null tensor"));
    return;
  }

  TArray<float> SrcElements;
  for (int i = 0; i < BBox.Landmark.Num(); i++) {
    SrcElements.Push(BBox.Landmark[i].Point.X);
    SrcElements.Push(BBox.Landmark[i].Point.Y);
  }

  if (Type() != EBandTensorType::UInt8) {
    const UEnum* EnumPtr =
        FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
    switch (Type()) {
      case EBandTensorType::Float32: {
        BandTensorUtil::FromTArray<float, float>(
            SrcElements, reinterpret_cast<float*>(Data()), NumElements());
        break;
      }

      default:
        UE_LOG(LogBand, Display,
               TEXT("FromCameraFrame: Failed to convert from %s"),
               *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
        break;
    }
  }
}

template <typename T>
std::pair<size_t, T> TemplatedArgMax(T* Buffer, size_t Length) {
  std::pair<size_t, T> BestIndexValue = {0, std::numeric_limits<T>::min()};
  for (size_t i = 0; i < Length; i++) {
    if (Buffer[i] > BestIndexValue.second) {
      BestIndexValue = {i, Buffer[i]};
    }
  }
  return BestIndexValue;
}

void UBandTensor::ArgMax(int32& Index, float& Value) {
  switch (Type()) {
    case EBandTensorType::Float32:
      std::tie(Index, Value) = TemplatedArgMax<float>(
          reinterpret_cast<float*>(Data()), NumElements());
      break;
    case EBandTensorType::Int32:
      std::tie(Index, Value) = TemplatedArgMax<int32>(
          reinterpret_cast<int32*>(Data()), NumElements());
      break;
    case EBandTensorType::UInt8:
      std::tie(Index, Value) = TemplatedArgMax<uint8>(
          reinterpret_cast<uint8*>(Data()), NumElements());
      break;
    case EBandTensorType::Int64:
      std::tie(Index, Value) = TemplatedArgMax<int64>(
          reinterpret_cast<int64*>(Data()), NumElements());
      break;
    case EBandTensorType::Int16:
      std::tie(Index, Value) = TemplatedArgMax<int16>(
          reinterpret_cast<int16*>(Data()), NumElements());
      break;
    case EBandTensorType::Complex64:
      std::tie(Index, Value) = TemplatedArgMax<double>(
          reinterpret_cast<double*>(Data()), NumElements());
      break;
    case EBandTensorType::Int8:
      std::tie(Index, Value) =
          TemplatedArgMax<int8>(reinterpret_cast<int8*>(Data()), NumElements());
      break;
    case EBandTensorType::Float64:
      std::tie(Index, Value) = TemplatedArgMax<double>(
          reinterpret_cast<double*>(Data()), NumElements());
      break;
    default:
      const UEnum* EnumPtr =
          FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
      UE_LOG(LogBand, Error, TEXT("ArgMax: Unsupported tensor type %s"),
             *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
      break;
  }
}

void UBandTensor::Initialize(BandTensor* NewTensorHandle) {
  TensorHandle = NewTensorHandle;
  const UEnum* EnumPtr =
      FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
  FString DimensionString;
  for (int32 i = 0; i < NumDims(); i++) {
    DimensionString += FString::FromInt(Dim(i));
    if (i < NumDims() - 1) {
      DimensionString += "x";
    }
  }
  RGBBuffer = new uint8[NumElements()];
}

EBandStatus UBandTensor::CopyFromFrameBuffer(
    std::unique_ptr<band::FrameBuffer> Src, FBandBoundingBox RoI,
    const float Mean, const float Std) {
  std::unique_ptr<band::FrameBufferUtils> utils =
      band::FrameBufferUtils::Create(
          band::FrameBufferUtils::ProcessEngine::kLibyuv);
  // BWHC format
  const int tensor_width = Dim(1);
  const int tensor_height = Dim(2);

  // Directly update uint8 Buffer
  uint8* TargetBufferPtr =
      Type() == EBandTensorType::UInt8 ? Data() : RGBBuffer;
  std::unique_ptr<band::FrameBuffer> OutputBuffer =
      band::CreateFromRgbRawBuffer(TargetBufferPtr,
                                   {tensor_width, tensor_height});

  // Get BoundingBox to put to FrameBufferUtils::Preprocess
  band::BoundingBox target_bbox;
  if (RoI == FBandBoundingBox()) {
    target_bbox.origin_x = 0;
    target_bbox.origin_y = 0;
    target_bbox.width = Src->dimension().width;
    target_bbox.height = Src->dimension().height;
  } else {
    target_bbox.origin_x = std::min(RoI.Position.Left, RoI.Position.Right) *
                           Src->dimension().width;
    target_bbox.origin_y = std::min(RoI.Position.Top, RoI.Position.Bottom) *
                           Src->dimension().height;
    target_bbox.width = std::abs(RoI.Position.Right - RoI.Position.Left) *
                        Src->dimension().width;
    target_bbox.height = std::abs(RoI.Position.Bottom - RoI.Position.Top) *
                         Src->dimension().height;
  }

  // TODO: we might need to crop and resize while preserving aspect ratio of the input RoI
  // Image preprocessing
  if (!utils->Preprocess(*Src, target_bbox, OutputBuffer.get(), true)) {
    UE_LOG(LogBand, Display, TEXT("FromCameraFrame: Failed to preprocess"));
    return EBandStatus::Error;
  }

  // Type conversion (RGB8 to tensor type)
  if (Type() != EBandTensorType::UInt8) {
    const UEnum* EnumPtr =
        FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
    switch (Type()) {
      case EBandTensorType::Float32:
        BandTensorUtil::RGB8ToRGBArray<float>(
            TargetBufferPtr, reinterpret_cast<float*>(Data()),
            tensor_width * tensor_height, Mean, Std);
        break;
      default:
        UE_LOG(LogBand, Display,
               TEXT("FromCameraFrame: Failed to convert from %s"),
               *EnumPtr->GetNameStringByValue(static_cast<int64>(Type())));
        return EBandStatus::Error;
    }
  }

  return EBandStatus::Ok;
}

EBandTensorType UBandTensor::Type() {
  return EBandTensorType(FBandModule::Get().BandTensorGetType(TensorHandle));
}

int32 UBandTensor::Dim(int32 Index) {
  if (Index < NumDims() && Index >= 0) {
    return FBandModule::Get().BandTensorGetDims(TensorHandle)[Index];
  } else {
    UE_LOG(LogBand, Error, TEXT("Dim: out of Index %d"), Index);
    return -1;
  }
}

int32 UBandTensor::NumDims() {
  return FBandModule::Get().BandTensorGetNumDims(TensorHandle);
}

int32 UBandTensor::NumElements() {
  int32 NumElements = 1;
  for (int i = 0; i < NumDims(); i++) {
    NumElements *= Dim(i);
  }
  return NumElements;
}

int32 UBandTensor::ByteSize() {
  return (int32)FBandModule::Get().BandTensorGetBytes(TensorHandle);
}

uint8* UBandTensor::Data() {
  return (uint8*)FBandModule::Get().BandTensorGetData(TensorHandle);
}

FString UBandTensor::Name() {
  return FString(FBandModule::Get().BandTensorGetName(TensorHandle));
}

TArray<uint8> UBandTensor::GetUInt8Buffer() { return {Data(), ByteSize()}; }

TArray<float> UBandTensor::GetF32Buffer() {
  return TArray<float>((float*)(Data()), ByteSize() / sizeof(float));
}

EBandStatus UBandTensor::CopyFromBuffer(uint8* Buffer, int32 bytes) {
  if (bytes != ByteSize()) {
    UE_LOG(LogBand, Error,
           TEXT("CopyFromBuffer: Buffer bytes %d != target tensor bytes %d"),
           bytes, ByteSize());
    return EBandStatus::Error;
  }
  memcpy(Data(), Buffer, bytes);
  return EBandStatus::Ok;
}

EBandStatus UBandTensor::CopyFromBuffer(TArray<uint8> Buffer) {
  if (ByteSize() != Buffer.GetAllocatedSize()) {
    UE_LOG(LogBand, Error,
           TEXT("CopyFromBuffer: Buffer bytes %llu != target tensor bytes %d"),
           Buffer.GetAllocatedSize(), ByteSize());
    return EBandStatus::Error;
  }
  memcpy(Data(), Buffer.GetData(), Buffer.GetAllocatedSize());
  return EBandStatus::Ok;
}

EBandStatus UBandTensor::CopyFromTextureWithCrop(UPARAM(ref)
    UTexture2D* Texture,
    FBandBoundingBox RoI,
    float Mean, float Std) {
  SCOPE_CYCLE_COUNTER(STAT_BandTextureToTensor);
  if (!Texture->PlatformData->Mips.Num()) {
    UE_LOG(LogBand, Error,
           TEXT("CopyFromTexture: No available mips from Texture"));
    return EBandStatus::Error;
  }

  bool Processed = true;
  auto task_reference = FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
    bool ChangedTexture2d = false;
    bool PreviousSrgb = Texture->SRGB;
    TextureCompressionSettings PreviousCompressionSettings =
        Texture->CompressionSettings;

    auto CleanUp = [&]() {
      if (ChangedTexture2d) {
        Texture->SRGB = PreviousSrgb;
        Texture->CompressionSettings = PreviousCompressionSettings;
        Texture->UpdateResource();
      }
    };

    if ((PreviousSrgb != false) ||
        (PreviousCompressionSettings != TC_VectorDisplacementmap)) {
      ChangedTexture2d = true;
      Texture->SRGB = false;
      Texture->CompressionSettings = TC_VectorDisplacementmap;
      Texture->UpdateResource();
    }

    const EBandTensorType TensorType = Type();
    const size_t TypeBytes = BandEnum::TensorTypeBytes(TensorType);
    const int32 SizeX = Texture->PlatformData->Mips[0].SizeX;
    const int32 SizeY = Texture->PlatformData->Mips[0].SizeY;

    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
    const size_t NumTensorElements = ByteSize() / 3 / TypeBytes;
    const size_t NumTextureElements = SizeX * SizeY;

    EPixelFormat TargetPixelFormat = Texture->PlatformData->PixelFormat;

    // resize or crop afterwards
    const bool RequiresResize =
        (NumTensorElements != NumTextureElements) || !(
          RoI == FBandBoundingBox());
    const uint8* SourceData =
        static_cast<const uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
    if (!SourceData) {
      UE_LOG(LogBand, Error,
             TEXT("CopyFromTexture: Tried to access null source data"));
      Processed = false;
    } else {
      // directly copy from the source to tensor
      if (!RequiresResize) {
        const UEnum* EnumPtr =
            FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);

        switch (TensorType) {
          case EBandTensorType::Float32:
            UE_LOG(
                LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"),
                *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
            BandTensorUtil::TextureToRGBArray<float>(
                SourceData, TargetPixelFormat, reinterpret_cast<float*>(Data()),
                NumTensorElements, Mean, Std);
            break;
          case EBandTensorType::UInt8:
            UE_LOG(
                LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"),
                *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
            BandTensorUtil::TextureToRGBArray<uint8>(
                SourceData, TargetPixelFormat, Data(), NumTensorElements, Mean,
                Std);
            break;
          case EBandTensorType::Int8:
            UE_LOG(
                LogBand, Log, TEXT("CopyFromTexture: EBandTensorType: %s"),
                *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
            BandTensorUtil::TextureToRGBArray<int8>(
                SourceData, TargetPixelFormat,
                reinterpret_cast<int8_t*>(Data()),
                NumTensorElements, Mean, Std);
            break;
          default:
            UE_LOG(
                LogBand, Error,
                TEXT("CopyFromTexture: Unsupported tensor type %s"),
                *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
            Processed = false;
            break;
        }
      } else {
        std::unique_ptr<band::FrameBuffer> Buffer =
            band::CreateFromRgbaRawBuffer(SourceData, {SizeX, SizeY});
        if (CopyFromFrameBuffer(std::move(Buffer), RoI, Mean, Std) ==
            EBandStatus::Error) {
          Processed = false;
        }
      }
    }
    Mip.BulkData.Unlock();
    CleanUp();
  }, TStatId(), nullptr, ENamedThreads::GameThread);
  task_reference->Wait();
  return Processed ? EBandStatus::Ok : EBandStatus::Error;
}

EBandStatus UBandTensor::CopyFromTexture(UPARAM(ref) UTexture2D* Texture,
                                         float Mean, float Std) {
  return CopyFromTextureWithCrop(Texture, {}, Mean, Std);
}

EBandStatus UBandTensor::CopyToBuffer(TArray<uint8> Buffer) {
  if (ByteSize() != Buffer.GetAllocatedSize()) {
    UE_LOG(LogBand, Error,
           TEXT("CopyToBuffer: Buffer bytes %llu != target tensor bytes %d"),
           Buffer.GetAllocatedSize(), ByteSize());
    return EBandStatus::Error;
  }
  memcpy(Buffer.GetData(), Data(), Buffer.GetAllocatedSize());
  return EBandStatus::Ok;
}

BandTensor* UBandTensor::Handle() const { return TensorHandle; }
