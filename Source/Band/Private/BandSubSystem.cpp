// Fill out your copyright notice in the Description page of Project Settings.

#include "BandSubsystem.h"
#include "Band.h"

void UBandSubSystem::Initialize(FSubsystemCollectionBase &Collection) {
  FBandModule::Get().RegisterSubSystem(this);
  Super::Initialize(Collection);
}

void UBandSubSystem::Deinitialize() {
  FBandModule::Get().UnregisterSubSystem(this);
  Super::Deinitialize();
}

void UBandSubSystem::OnEndInvokeInternal(int32 JobId, BandStatus Status) const {
  AsyncTask(ENamedThreads::GameThread, [&, JobId, Status]() {
    OnEndInvokeDynamic.Broadcast(JobId, BandEnum::ToBandStatus(Status));
  });
  OnEndInvoke.Broadcast(JobId, BandEnum::ToBandStatus(Status));
}