#include "BandEnum.h"

namespace BandEnum {
size_t TensorTypeBytes(const EBandTensorType& Type) {
  size_t Size = 0;
  switch (Type) {
    case EBandTensorType::NoType:
      Size = 0;
      break;
    case EBandTensorType::Float32:
      Size = sizeof(float);
      break;
    case EBandTensorType::Int32:
      Size = sizeof(int32);
      break;
    case EBandTensorType::UInt8:
      Size = sizeof(uint8);
      break;
    case EBandTensorType::Int64:
      Size = sizeof(int64);
      break;
    case EBandTensorType::String:
      Size = sizeof(char);
      break;
    case EBandTensorType::Bool:
      Size = sizeof(bool);
      break;
    case EBandTensorType::Int16:
      Size = sizeof(int16);
      break;
    case EBandTensorType::Complex64:
      Size = sizeof(double);
      break;
    case EBandTensorType::Int8:
      Size = sizeof(int8);
      break;
    case EBandTensorType::Float16:
      Size = 2;
      break;
    case EBandTensorType::Float64:
      Size = sizeof(double);
      break;
    default:
      break;
  }
  return Size;
}

EBandStatus ToBandStatus(BandStatus Status) {
  return static_cast<EBandStatus>(Status);
}
} // namespace BandEnum
