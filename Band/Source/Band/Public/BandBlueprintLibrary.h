#pragma once

#include "Band.h"
#include "BandTensor.h"
#include "BandEnum.h"
#include "BandLabel.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

class UBandBoundingBox;
UCLASS()
class BAND_API UBandBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Band")
	static FString GetVersion();
	
	UFUNCTION(BlueprintCallable, Category = "Band")
	static UBandInterpreter* GetInterpreter();

	UFUNCTION(BlueprintCallable, Category = "Band")
	static EBandStatus Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor *> OutputTensors);
	
	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static TArray<UBandBoundingBox*> GetDetectedBoxes(UPARAM(ref) TArray<UBandTensor *> OutputTensors, EBandDetector DetectorType, UBandLabel* Label = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Band | Postprocessing")
	static TArray<UBandBoundingBox*> NMS(TArray<UBandBoundingBox *> Boxes, const float IoU_Threshold);
};