// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <unordered_map>

#include "CoreMinimal.h"
#include "BandEnum.h"
#include "BandLibraryWrapper.h"
#include "BandInterpreterComponent.generated.h"

class UBandModel;
class UBandTensor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndInvokeDynamic, int32, JobId, EBandStatus, InvokeStatus);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndInvoke, int32, EBandStatus);
/**
 *
 * Interpreter object that act as interface for Band Library.
 * Internally utilizes global interpreter handle that
 * FBandModule manages.
 */
UCLASS(ClassGroup = (Band), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class BAND_API UBandInterpreterComponent : public UActorComponent
{
public:
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 GetInputTensorCount(UPARAM(ref) UBandModel* Model);
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 GetOutputTensorCount(UPARAM(ref) UBandModel* Model);
	UFUNCTION(BlueprintCallable, Category = "Band")
	UBandTensor* AllocateInputTensor(UPARAM(ref) UBandModel* Model, int32 InputIndex);
	UFUNCTION(BlueprintCallable, Category = "Band")
	UBandTensor* AllocateOutputTensor(UPARAM(ref) UBandModel* Model, int32 OutputIndex);

	UFUNCTION(BlueprintCallable, Category = "Band")
	void InvokeSync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors);
	UFUNCTION(BlueprintCallable, Category = "Band")
	int32 InvokeAsync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors);

	UFUNCTION(BlueprintCallable, Category = "Band")
	EBandStatus Wait(int32 JobId, UPARAM(ref) TArray<UBandTensor*> OutputTensors);

	UPROPERTY(BlueprintAssignable, Category = Band)
	FOnEndInvokeDynamic OnEndInvokeDynamic;
	/*
	CAUTION: This delegate doesn't guarantee access from game thread
	*/
	FOnEndInvoke OnEndInvoke;
	
private:
	void OnEndInvokeInternal(int JobId, TfLiteStatus Status) const;
	
	Band::TfLiteInterpreter* GetHandle() const;

	// <Job Id, Model Handle>
	// Temporal solution for output tensor length validation
	mutable std::unordered_map<int32, int32> JobToModel;
	friend class FBandModule;
};
