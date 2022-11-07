#pragma once

#include "common.h"
#include "Modules/ModuleManager.h"
#include "BandEnum.generated.h"

/*
	UEnum for TfLiteType in common.h
*/
UENUM(BlueprintType)
enum class EBandTensorType : uint8
{
	NoType UMETA(DisplayName = "NoType"),
	Float32 UMETA(DisplayName = "Float32"),
	Int32 UMETA(DisplayName = "Int32"),
	UInt8 UMETA(DisplayName = "UInt8"),
	Int64 UMETA(DisplayName = "Int64"),
	String UMETA(DisplayName = "String"),
	Bool UMETA(DisplayName = "Bool"),
	Int16 UMETA(DisplayName = "Int16"),
	Complex64 UMETA(DisplayName = "Complex64"),
	Int8 UMETA(DisplayName = "Int8"),
	Float16 UMETA(DisplayName = "Float16"),
	Float64 UMETA(DisplayName = "Float64"),
};

/*
	UEnum for BandStatus in common.h
*/
UENUM(BlueprintType)
enum class EBandStatus : uint8
{
	Ok UMETA(DisplayName = "Ok"),
	Error UMETA(DisplayName = "Error"),
	DelegateError UMETA(DisplayName = "DelegateError"),
};

UENUM(BlueprintType)
enum class EBandDetector : uint8
{
	RetinaFace UMETA(DisplayName = "RetinaFace"),
	PalmDetection UMETA(DisplayName = "PalmDetection"),
	SSD UMETA(DisplayName = "SSD"),
	Unknown UMETA(DisplayName = "Unknown")
};

namespace BandEnum
{
	size_t TensorTypeBytes(const EBandTensorType& Type);
	EBandStatus ToBandStatus(BandStatus Status);
} // namespace BandEnum