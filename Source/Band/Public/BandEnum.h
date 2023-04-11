#pragma once

#include "c_api_types.h"
#include "Modules/ModuleManager.h"
#include "BandEnum.generated.h"

/*
	UEnum for TfLiteType in common.h
*/
UENUM(BlueprintType)
enum class EBandTensorType : uint8 {
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

UENUM(BlueprintType)
enum class EBandDeviceType : uint8 {
  CPU UMETA(DisplayName = "CPU"),
  GPU UMETA(DisplayName = "GPU"),
  DSP UMETA(DisplayName = "DSP"),
  NPU UMETA(DisplayName = "NPU"),
};

/*
	UEnum for BandStatus in common.h
*/
UENUM(BlueprintType)
enum class EBandStatus : uint8 {
  Ok UMETA(DisplayName = "Ok"),
  Error UMETA(DisplayName = "Error"),
  DelegateError UMETA(DisplayName = "DelegateError"),
};

UENUM(BlueprintType)
enum class EBandDetector : uint8 {
  RetinaFace UMETA(DisplayName = "RetinaFace"),
  PalmDetection UMETA(DisplayName = "PalmDetection"),
  SSD UMETA(DisplayName = "SSD"),
  SSDMNetV2 UMETA(DisplayName = "SSDMobileNetV2"),
  Unknown UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EBandLandmark : uint8 {
  MoveNetSingleThunder UMETA(DisplayName = "SinglePoseMovenetThunder"),
  MoveNetSingleLightning UMETA(DisplayName = "SinglePoseMovenetLightning"),
  HandLandmarkMediapipe UMETA(DIsplayName = "HandLandmarkMediapipe"),
  FaceMeshMediapipe UMETA(DIsplayName = "FaceMeshMediapipe"),
  Unknown UMETA(DisplayName = "Unknown")
};

namespace BandEnum {
size_t TensorTypeBytes(const EBandTensorType& Type);
EBandStatus ToBandStatus(BandStatus Status);
} // namespace BandEnum
