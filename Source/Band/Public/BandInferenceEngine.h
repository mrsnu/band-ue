// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <mutex>
#include <queue>

#include "CoreMinimal.h"
#include "BandInterfaceComponent.h"
#include "GameFramework/Actor.h"
#include "BandInferenceEngine.generated.h"

class UAndroidCameraFrame;
class UAndroidCameraComponent;
class UBandModel;
class UBandTensor;
class UBandLabel;
class UTextBlock;

UCLASS()
class BAND_API ABandInferenceEngine : public AActor {
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ABandInferenceEngine();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  void OnFrameAvailable(const UAndroidCameraFrame* CameraFrame);
  void OnBeginFrame();
  void OnEndFrame();
  void OnBeginFrameRT();
  void OnEndFrameRT();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
  UBandModel* Model = nullptr;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Band")
  UBandLabel* Label = nullptr;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
  TSubclassOf<class UUserWidget> UIWidgetReference;

  UUserWidget* UIWidget;
  UAndroidCameraComponent* CameraComponent = nullptr;
  UBandInterfaceComponent* Interpreter = nullptr;

  uint64 BeginFrame = 0;
  uint64 BeginFrameRT = 0;

  std::mutex CameraMutex;
  uint64 BeginCameraFrame = 0;
  std::queue<double> BeginCameraFrames;
  bool RequiresCameraReport = false;

  FString Main, Render, DNN;

  UTextBlock* ClassTextBlock;
  UTextBlock* MainTextBlock;
  UTextBlock* RenderTextBlock;
  UTextBlock* DNNTextBlock;

  FDelegateHandle OnBeginFrameHandle;
  FDelegateHandle OnFrameHandle;
  FDelegateHandle OnEndFrameHandle;
  FDelegateHandle OnBeginFrameRTHandle;
  FDelegateHandle OnEndFrameRTHandle;

  TArray<UBandTensor*> InputTensors;
  TArray<UBandTensor*> OutputTensors;

};
