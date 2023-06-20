// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <unordered_map>

#include "BandEnum.h"
#include "BandSubsystem.generated.h"
#include "CoreMinimal.h"
#include "libband.h"


class UBandModel;
class UBandTensor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndInvokeDynamic, int32, JobId,
                                             EBandStatus, InvokeStatus);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndInvoke, int32, EBandStatus);
/**
 *
 * Interpreter object that act as interface for Band Library.
 * Internally utilizes global interpreter handle that
 * FBandModule manages.
 */
UCLASS(ClassGroup = (Band), NotBlueprintable)
class BAND_API UBandSubSystem : public UGameInstanceSubsystem {
public:
  GENERATED_BODY()

  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;

  UPROPERTY(BlueprintAssignable, Category = Band)
  FOnEndInvokeDynamic OnEndInvokeDynamic;
  /*
  CAUTION: This delegate doesn't guarantee access from game thread
  */
  FOnEndInvoke OnEndInvoke;

private:
  friend class FBandModuleImpl;
  void OnEndInvokeInternal(int JobId, BandStatus Status) const;

  friend class FBandModule;
};
