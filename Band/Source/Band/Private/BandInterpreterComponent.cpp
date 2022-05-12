// Fill out your copyright notice in the Description page of Project Settings.


#include "BandInterpreterComponent.h"
#include "BandModel.h"
#include "BandTensor.h"
#include "Band.h"
#include "BandTensorUtil.h"


void UBandInterpreterComponent::BeginPlay()
{
	// Reserve enough amount to avoid run-time realloc
	JobToModel.reserve(1000);
	FBandModule::Get().RegisterInterpreter(this);
	Super::BeginPlay();
}

void UBandInterpreterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FBandModule::Get().UnregisterInterpreter(this);
	Super::EndPlay(EndPlayReason);
}

int32 UBandInterpreterComponent::GetInputTensorCount(UPARAM(ref) UBandModel* Model)
{
	// Skip model registration check (lazily register in get handle)
	return FBandModule::Get().TfLiteInterpreterGetInputTensorCount(GetHandle(), Model->GetHandle());
}

int32 UBandInterpreterComponent::GetOutputTensorCount(UPARAM(ref) UBandModel* Model)
{
	// Skip model registration check (lazily register in get handle)
	return FBandModule::Get().TfLiteInterpreterGetOutputTensorCount(GetHandle(), Model->GetHandle());
}

UBandTensor* UBandInterpreterComponent::AllocateInputTensor(UPARAM(ref) UBandModel* Model, int32 InputIndex)
{
	// Skip model registration check (lazily register in get handle)
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->Initialize(FBandModule::Get().TfLiteInterpreterAllocateInputTensor(GetHandle(), Model->GetHandle(), InputIndex));
	return Tensor;
}

UBandTensor* UBandInterpreterComponent::AllocateOutputTensor(UPARAM(ref) UBandModel* Model, int32 OutputIndex)
{
	// Skip model registration check (lazily register in get handle)
	UBandTensor* Tensor = NewObject<UBandTensor>();
	Tensor->Initialize(FBandModule::Get().TfLiteInterpreterAllocateOutputTensor(GetHandle(), Model->GetHandle(), OutputIndex));
	return Tensor;
}

void UBandInterpreterComponent::InvokeSync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	if (Model->IsRegistered() && GetInputTensorCount(Model) == InputTensors.Num() && GetOutputTensorCount(Model) == OutputTensors.Num())
	{
		FBandModule::Get().TfLiteInterpreterInvokeSync(GetHandle(), Model->GetHandle(), BandTensorUtil::TensorsFromTArray(InputTensors).GetData(), BandTensorUtil::TensorsFromTArray(OutputTensors).GetData());	
	}
	else
	{
		UE_LOG(LogBand, Error, TEXT("InvokeSync: Failed to invoke. Model registered %d Input Count (Expected %d, Given %d) Output Count (Expected %d, Given %d)"),
			Model->IsRegistered(), GetInputTensorCount(Model) , InputTensors.Num() , GetOutputTensorCount(Model) , OutputTensors.Num());
	}

}

int32 UBandInterpreterComponent::InvokeAsync(UPARAM(ref) UBandModel* Model, UPARAM(ref) TArray<UBandTensor*> InputTensors)
{
	if (Model->IsRegistered() && GetInputTensorCount(Model) == InputTensors.Num()) {
		const int32 JobId = FBandModule::Get().TfLiteInterpreterInvokeAsync(GetHandle(), Model->GetHandle(), BandTensorUtil::TensorsFromTArray(InputTensors).GetData());
		JobToModel[JobId] = Model->GetHandle();
		return JobId;
	}
	else
	{
		UE_LOG(LogBand, Error, TEXT("InvokeSync: Failed to invoke. Model registered %d Input Count (Expected %d, Given %d)"),
			Model->IsRegistered(), GetInputTensorCount(Model) , InputTensors.Num());
		return -1;
	}
}

EBandStatus UBandInterpreterComponent::Wait(int32 JobId, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	// TODO(dostos): Add `GetJob(int32 jobId)` to C API?
	const int32 OutputTensorCount = FBandModule::Get().TfLiteInterpreterGetOutputTensorCount(GetHandle(), JobToModel[JobId]);
	if (OutputTensors.Num() == OutputTensorCount)
	{
		return static_cast<EBandStatus>(FBandModule::Get().TfLiteInterpreterWait(GetHandle(), JobId, BandTensorUtil::TensorsFromTArray(OutputTensors).GetData()));
	}
	else
	{
		UE_LOG(LogBand, Error, TEXT("InvokeSync: Failed to invoke. Output Count (Expected %d, Given %d)"),
			OutputTensors.Num() , OutputTensorCount);
		return EBandStatus::Error;
	}
}

void UBandInterpreterComponent::OnEndInvokeInternal(int32 JobId, TfLiteStatus Status) const
{
	AsyncTask(ENamedThreads::GameThread, [&, JobId, Status]() {
		UE_LOG(LogBand, Display, TEXT("Finished Job id %d."), JobId);
		OnEndInvokeDynamic.Broadcast(JobId, BandEnum::ToBandStatus(Status));
		JobToModel.erase(JobId);
	});
	OnEndInvoke.Broadcast(JobId, BandEnum::ToBandStatus(Status));
}

Band::TfLiteInterpreter* UBandInterpreterComponent::GetHandle() const
{
	return FBandModule::Get().GetInterpreterHandle();
}
