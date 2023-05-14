// Fill out your copyright notice in the Description page of Project Settings.

#include "BandInferenceEngine.h"
#include "Band.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "BandLabel.h"
#include "AndroidCamera.h"
#include "BandTrace.h"
#include "AndroidCameraFrame.h"
#include "AndroidCameraComponent.h"

// Widget classes
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

// Core delegates
#include "Misc/CoreDelegates.h"

// Sets default values
ABandInferenceEngine::ABandInferenceEngine() {
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = false;
  Interpreter = CreateDefaultSubobject<UBandInterfaceComponent>(
      TEXT("InterpreterComponent"));
  CameraComponent = CreateDefaultSubobject<UAndroidCameraComponent>(
      TEXT("CameraComponent"));
}

// Called when the game starts or when spawned
void ABandInferenceEngine::BeginPlay() {
  if (Model && Label) {
    if (UIWidgetReference) {
      UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetReference);
      UIWidget->AddToViewport();
      ClassTextBlock = Cast<UTextBlock>(
          UIWidget->GetWidgetFromName("ClassLabel"));
      MainTextBlock = Cast<UTextBlock>(
          UIWidget->GetWidgetFromName("MainThread"));
      RenderTextBlock = Cast<UTextBlock>(
          UIWidget->GetWidgetFromName("RenderThread"));
      DNNTextBlock = Cast<UTextBlock>(UIWidget->GetWidgetFromName("DNNThread"));
    }

    OnFrameHandle = CameraComponent->OnFrameAvailable.AddUObject(
        this, &ABandInferenceEngine::OnFrameAvailable);
    OnBeginFrameHandle = FCoreDelegates::OnBeginFrame.AddUObject(
        this, &ABandInferenceEngine::OnBeginFrame);
    OnEndFrameHandle = FCoreDelegates::OnEndFrame.AddUObject(
        this, &ABandInferenceEngine::OnEndFrame);
    OnBeginFrameRTHandle = FCoreDelegates::OnBeginFrameRT.AddUObject(
        this, &ABandInferenceEngine::OnBeginFrameRT);
    OnEndFrameRTHandle = FCoreDelegates::OnEndFrameRT.AddUObject(
        this, &ABandInferenceEngine::OnEndFrameRT);

    // Allocate tensors
    InputTensors.Add(Interpreter->AllocateInputTensor(Model, 0));
    OutputTensors.Add(Interpreter->AllocateOutputTensor(Model, 0));

    const int Width = InputTensors[0]->Dim(1);
    const int Height = InputTensors[0]->Dim(2);

    UE_LOG(LogBand, Log, TEXT("Start camera desired size %d x %d"), Width,
           Height);

    CameraComponent->StartCamera(Width, Height);
  } else {
    UE_LOG(LogBand, Error, TEXT("Failed to start inference engine"));
  }

  Super::BeginPlay();
}

void ABandInferenceEngine::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  if (CameraComponent) {
    CameraComponent->OnFrameAvailable.Remove(OnFrameHandle);
  }
  // Remove callback handles
  FCoreDelegates::OnBeginFrame.Remove(OnBeginFrameHandle);
  FCoreDelegates::OnEndFrame.Remove(OnEndFrameHandle);
  FCoreDelegates::OnBeginFrameRT.Remove(OnBeginFrameRTHandle);
  FCoreDelegates::OnEndFrameRT.Remove(OnEndFrameRTHandle);

  Super::EndPlay(EndPlayReason);
}

void ABandInferenceEngine::OnFrameAvailable(
    const UAndroidCameraFrame* CameraFrame) {
  if (!CameraFrame) {
    UE_LOG(LogBand, Display, TEXT("OnFrameAvailable on null CameraFrame"));
    return;
  }
  UE_SCOPED_BANDTIMER(OnFrameAvailable);
  // Assumption: FIFO
  {
    std::lock_guard<std::mutex> Lock(CameraMutex);
    BeginCameraFrame = FPlatformTime::Cycles64();
    BeginCameraFrames.push(FPlatformTime::Cycles64());
  }
  // Temporal task: UI Update for camera feed
  AsyncTask(ENamedThreads::GameThread, [CameraFrame, this]() {
    UE_SCOPED_BANDTIMER(UpdateWidget);
    auto Texture2D = CameraFrame->GetTexture2D();
    // Update Widget
    auto ImageWidget = Cast<UImage>(UIWidget->GetWidgetFromName("CameraImage"));
      ImageWidget->SetBrushFromTexture(Texture2D, true);
      {
    }
  });
  {
    UE_SCOPED_BANDTIMER(InputPreprocess);
    InputTensors[0]->FromCameraFrame(CameraFrame, false);
  }

  {
    UE_SCOPED_BANDTIMER(Inference);
    Interpreter->InvokeSync(Model, InputTensors, OutputTensors);
  }

  int ClassIndex = -1;
  float MaxValue = -1;
  OutputTensors[0]->ArgMax(ClassIndex, MaxValue);

  {
    // Temporal task: UI Update for camera feed
    AsyncTask(ENamedThreads::GameThread, [&]() {
      std::lock_guard<std::mutex> Lock(CameraMutex);
      if (ClassIndex != -1) {
        ClassTextBlock->SetText(
            FText::FromString(Label->GetClassName(ClassIndex)));
      }
      RequiresCameraReport = true;
    });
  }
}

void ABandInferenceEngine::OnBeginFrame() {
  BeginFrame = FPlatformTime::Cycles64();
}

void ABandInferenceEngine::OnEndFrame() {
  if (MainTextBlock) {
    double FrameTime = FPlatformTime::ToMilliseconds64(
        FPlatformTime::Cycles64() - BeginFrame);
    AsyncTask(ENamedThreads::GameThread, [&, FrameTime]() {
      MainTextBlock->SetText(
          FText::FromString(
              FString::Printf(TEXT("Main thread %f ms"), FrameTime)));
    });

    std::lock_guard<std::mutex> Lock(CameraMutex);
    if (RequiresCameraReport && BeginCameraFrames.size()) {
      double CameraTime = FPlatformTime::ToMilliseconds64(
          FPlatformTime::Cycles64() - BeginCameraFrame);
      AsyncTask(ENamedThreads::GameThread, [&, CameraTime]() {
        DNNTextBlock->SetText(FText::FromString(
            FString::Printf(TEXT("DNN processing thread %f ms"), CameraTime)));
      });
      RequiresCameraReport = false;
    }
  }
}

void ABandInferenceEngine::OnBeginFrameRT() {
  BeginFrameRT = FPlatformTime::Cycles64();
}

void ABandInferenceEngine::OnEndFrameRT() {
  if (RenderTextBlock) {
    double FrameTime = FPlatformTime::ToMilliseconds64(
        FPlatformTime::Cycles64() - BeginFrameRT);
    AsyncTask(ENamedThreads::GameThread, [&, FrameTime]() {
      RenderTextBlock->SetText(
          FText::FromString(
              FString::Printf(TEXT("Render thread %f ms"), FrameTime)));
    });
  }
}
