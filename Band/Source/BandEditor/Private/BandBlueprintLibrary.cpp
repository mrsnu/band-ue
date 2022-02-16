#include "BandBlueprintLibrary.h"
#include "Band.h"
#include "BandModel.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

int32 UBandBlueprintLibrary::GetInputTensorCount(UBandModel* Model)
{
	UE_LOG(LogTemp, Log, TEXT("Band model registered %d"), Model->GetModelHandle());
	return 0;// FBandModule::Get().GetInputTensorCount(Model);
}

int32 UBandBlueprintLibrary::GetOutputTensorCount(UBandModel* Model)
{
	return FBandModule::Get().GetOutputTensorCount(Model);
}