#include "BandEnum.h"

namespace BandEnum
{
	size_t TensorTypeBytes(const EBandTensorType& Type)
	{
		size_t Size = 0;
		switch (Type)
		{
		case EBandTensorType::NoType: Size = 0;
		case EBandTensorType::Float32: Size = sizeof(float);
		case EBandTensorType::Int32: Size = sizeof(int32);
		case EBandTensorType::UInt8: Size = sizeof(uint8);
		case EBandTensorType::Int64: Size = sizeof(int64);
		case EBandTensorType::String: Size = sizeof(char);
		case EBandTensorType::Bool: Size = sizeof(bool);
		case EBandTensorType::Int16: Size = sizeof(int16);
		case EBandTensorType::Complex64: Size = sizeof(double);
		case EBandTensorType::Int8: Size = sizeof(int8);
		case EBandTensorType::Float16: Size = 2;
		case EBandTensorType::Float64: Size = sizeof(double);
		default: break;
		}
		return Size;
	}
}