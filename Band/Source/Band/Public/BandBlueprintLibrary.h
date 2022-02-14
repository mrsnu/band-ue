#pragma once

#include "Band.h"
#include "BandBlueprintLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BandBlueprintLibrary.generated.h"

UCLASS()
class UBandBlueprintLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="Band")
	static void GetTfLiteVersion();
};