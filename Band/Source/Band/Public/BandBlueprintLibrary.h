#pragma once

#include "Band.h"
#include "BandBoundingBox.h"
#include "BandTensor.h"
#include "BandEnum.h"
#include "BandLabel.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Band")
	static FString GetVersion();
	
	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static TArray<FBandBoundingBox> GetDetectedBoxes(UPARAM(ref) TArray<UBandTensor *> OutputTensors, EBandDetector DetectorType, UBandLabel* Label = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static TArray<FBandBoundingBox> NMS(TArray<FBandBoundingBox> Boxes, const float IoU_Threshold);

	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static void PrintBox(FBandBoundingBox BoundingBox);
	
	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static void ParseRectF(FBandBoundingBox BoundingBox, const int ImageHeight, const int ImageWidth, float& PosX, float& PosY, float& SizeX, float& SizeY);
};