// Fill out your copyright notice in the Description page of Project Settings.


#include "BandInterpreter.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "Band.h"
#include "BandTensorUtil.h"


void ABandInterpreter::BeginPlay()
{
	FBandModule::Get().RegisterInterpreterActor(this);
	Super::BeginPlay();
}

void ABandInterpreter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FBandModule::Get().UnregisterInterpreterActor(this);
	Super::EndPlay(EndPlayReason);
}

int32 ABandInterpreter::GetInputTensorCount(UPARAM(ref) UBandModel* Model)
{
	// Skip model registration check (lazily register in get handle)
	return FBandModule::Get().TfLiteInterpreterGetInputTensorCount(GetHandle(), Model->GetHandle());
}

int32 ABandInterpreter::GetOutputTensorCount(UPARAM(ref) UBandModel* Model)
{
	// Skip model registration check (lazily register in get handle)
	return FBandModule::Get().TfLiteInterpreterGetOutputTensorCount(GetHandle(), Model->GetHandle());
}

UBandTensor* ABandInterpreter::AllocateInputTensor(UPARAM(ref) UBandModel* Model, int32 InputIndex)
{
	// Skip model registration check (lazily register in get handle)
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->Initialize(FBandModule::Get().TfLiteInterpreterAllocateInputTensor(GetHandle(), Model->GetHandle(), InputIndex));
	return Tensor;
}

UBandTensor* ABandInterpreter::AllocateOutputTensor(UPARAM(ref) UBandModel* Model, int32 OutputIndex)
{
	// Skip model registration check (lazily register in get handle)
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->Initialize(FBandModule::Get().TfLiteInterpreterAllocateOutputTensor(GetHandle(), Model->GetHandle(), OutputIndex));
	return Tensor;
}

void ABandInterpreter::InvokeSync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	check(Model->IsRegistered());
	FBandModule::Get().TfLiteInterpreterInvokeSync(GetHandle(), Model->GetHandle(), BandTensorUtil::TensorsFromTArray(InputTensors).GetData(), BandTensorUtil::TensorsFromTArray(OutputTensors).GetData());
}

int32 ABandInterpreter::InvokeAsync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors)
{
	check(Model->IsRegistered());
	return FBandModule::Get().TfLiteInterpreterInvokeAsync(GetHandle(), Model->GetHandle(), BandTensorUtil::TensorsFromTArray(InputTensors).GetData());
}

EBandStatus ABandInterpreter::Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	return EBandStatus(FBandModule::Get().TfLiteInterpreterWait(GetHandle(), JobHandle, BandTensorUtil::TensorsFromTArray(OutputTensors).GetData()));
}

void ABandInterpreter::OnEndInvokeInternal(int32 JobId, TfLiteStatus Status) const
{
	AsyncTask(ENamedThreads::GameThread, [&]() {
		OnEndInvokeDynamic.Broadcast(JobId, BandEnum::ToBandStatus(Status));
	});
	OnEndInvoke.Broadcast(JobId, BandEnum::ToBandStatus(Status));
}

Band::TfLiteInterpreter* ABandInterpreter::GetHandle() const
{
	return FBandModule::Get().GetInterpreterHandle();
}
