#include "BandBlueprintLibrary.h"
#include "Band.h"
#include "BandModel.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

EBandStatus UBandBlueprintLibrary::Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	return FBandModule::Get().Wait(JobHandle, OutputTensors);
}