#include "BandBlueprintLibrary.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary/BandLibrary.h"

FString UBandBlueprintLibrary::Version()
{
	const char* tfLiteVersion = TfLiteVersion();
	return FString(tfLiteVersion);
}
