#include "BandLibraryWrapper.h"
#include "Core.h"


#define LoadFunction(dllHandle, function) \
    function = reinterpret_cast<p##function>(FPlatformProcess::GetDllExport(dllHandle, L#function));

void LoadBandFunction(void* dllHandle) {
    LoadFunction(dllHandle, TfLiteVersion);
    LoadFunction(dllHandle, TfLiteModelCreate);
}