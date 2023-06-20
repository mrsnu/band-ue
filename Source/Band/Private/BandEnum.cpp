#include "BandEnum.h"

namespace BandEnum {
size_t TensorTypeBytes(const EBandTensorDataType &Type) {
  size_t Size = 0;
  switch (Type) {
  case EBandTensorDataType::NoType:
    Size = 0;
    break;
  case EBandTensorDataType::Float32:
    Size = sizeof(float);
    break;
  case EBandTensorDataType::Int32:
    Size = sizeof(int32);
    break;
  case EBandTensorDataType::UInt8:
    Size = sizeof(uint8);
    break;
  case EBandTensorDataType::Int64:
    Size = sizeof(int64);
    break;
  case EBandTensorDataType::String:
    Size = sizeof(char);
    break;
  case EBandTensorDataType::Bool:
    Size = sizeof(bool);
    break;
  case EBandTensorDataType::Int16:
    Size = sizeof(int16);
    break;
  case EBandTensorDataType::Complex64:
    Size = sizeof(double);
    break;
  case EBandTensorDataType::Int8:
    Size = sizeof(int8);
    break;
  case EBandTensorDataType::Float16:
    Size = 2;
    break;
  case EBandTensorDataType::Float64:
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
