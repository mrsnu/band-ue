#pragma once
#pragma once

#include "Modules/ModuleManager.h"
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
	UEnum for TfLiteStatus in common.h
*/
UENUM(BlueprintType)
enum class EBandStatus : uint8
{
	Ok UMETA(DisplayName = "Ok"),
	Error UMETA(DisplayName = "Error"),
	DelegateError UMETA(DisplayName = "DelegateError"),
};