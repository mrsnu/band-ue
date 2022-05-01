// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BandEnum.h"
#include "BandLibraryWrapper.h"
#include "UObject/Object.h"
#include "BandInterpreter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndInvokeDynamic, int, JobId, EBandStatus, InvokeStatus);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndInvoke, int, EBandStatus);
/**
 *
 * Interpreter object that Band module owns interpreter handle
 * Mainly serve as a global UObject for module to utilize
 * object-related features such as delegate
 */
UCLASS(NotBlueprintable)
class BAND_API UBandInterpreter : public UObject
{
public:
	GENERATED_BODY()
	void Initialize(Band::TfLiteInterpreter* Handle);

	Band::TfLiteInterpreter* GetHandle() const;
	operator Band::TfLiteInterpreter*() const;

	UPROPERTY(BlueprintAssignable, Category = Band)
	FOnEndInvokeDynamic OnEndInvokeDynamic;
	/*
	CAUTION: This delegate doesn't guarantee access from game thread
	*/
	FOnEndInvoke OnEndInvoke;
private:
	Band::TfLiteInterpreter* InterpreterHandle = nullptr;
};
