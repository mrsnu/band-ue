#pragma once

#include "Band.h"
#include "BandTensor.h"
#include "BandEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Band")
	static FString GetVersion();

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor *> OutputTensors);

	/*
	* - Based on the assumption that:
	*		Results.length = Tensor->NumDims() * OUTPUT_OFFSET
	* - BBoxOffset[0] = Offset of Confidence
	*   BBoxOffset[1~4] = Offset of Left, Bottom, Right, Top (in this order)
	*/
	UFUNCTION(BlueprintCallable, Category = "Band")
	static TArray<UBandBoundingBox*> GetDetectedBoxes(UPARAM(ref) UBandTensor* Tensor, TArray<float> Results, float SCORE_THRESHOLD, TArray<int32> BBoxOffsets);

	UFUNCTION(BlueprintCallable, Category = "Band")
	static TArray<UBandBoundingBox*> NMS(TArray<UBandBoundingBox *> Boxes, float IOU_THRESHOLD);
};