#pragma once

#include "Band.h"
#include "BandBoundingBox.h"
#include "BandEnum.h"
#include "BandLabel.h"
#include "BandLandmark.h"
#include "BandTensor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"


UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "Band")
  static FString GetVersion();

  UFUNCTION(BlueprintCallable, Category = "Band")
  static int32 GetNumWorkers();

  UFUNCTION(BlueprintCallable, Category = "Band")
  static EBandDeviceType GetWorkerDeviceType(int Index);

  UFUNCTION(BlueprintCallable, Category = "Band")
  static void RequestSync(UPARAM(ref) const UBandModel *Model,
                          UPARAM(ref) TArray<UBandTensor *> InputTensors,
                          UPARAM(ref) TArray<UBandTensor *> OutputTensors,
                          int WorkerIndex = 0);

  UFUNCTION(BlueprintCallable, Category = "Band")
  static int32 RequestAsync(UPARAM(ref) const UBandModel *Model,
                            UPARAM(ref) TArray<UBandTensor *> InputTensors,
                            int DeviceFlag = 0);

  UFUNCTION(BlueprintCallable, Category = "Band")
  static EBandStatus
  GetOutputs(int32 JobId, UPARAM(ref) TArray<UBandTensor *> OutputTensors);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static FString GetLabel(UPARAM(ref) TArray<UBandTensor *> OutputTensors,
                          UBandLabel *Label);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static TArray<FBandBoundingBox>
  GetDetectedBoxes(UPARAM(ref) TArray<UBandTensor *> OutputTensors,
                   EBandDetector DetectorType, UBandLabel *Label = nullptr);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static TArray<FVector2D>
  Get2DLandmarks(UPARAM(ref) TArray<UBandTensor *> OutputTensors,
                 EBandLandmark ModelType);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static FBandBoundingBox GetLandmarks(TArray<UBandTensor *> Tensors,
                                       EBandLandmark ModelType);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static TArray<FBandBoundingBox> NMS(TArray<FBandBoundingBox> Boxes,
                                      const float IoU_Threshold);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static TArray<FBandBoundingBox>
  FilterBoxesByConfidence(TArray<FBandBoundingBox> Boxes,
                          const int MaxNumBoxes);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static TArray<FBandBoundingBox>
  FilterBoxesByLabel(TArray<FBandBoundingBox> Boxes, FString Label);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static void PrintBox(FBandBoundingBox BoundingBox);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static FString BoxToString(FBandBoundingBox BoundingBox);

  UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
  static void ParseRectF(FBandBoundingBox BoundingBox, const int ImageHeight,
                         const int ImageWidth, float &PosX, float &PosY,
                         float &SizeX, float &SizeY);
};
