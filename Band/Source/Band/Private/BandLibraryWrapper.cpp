#include "BandLibraryWrapper.h"
#include "Core.h"


#define LoadFunction(dllHandle, function) \
    function = reinterpret_cast<p##function>(FPlatformProcess::GetDllExport(dllHandle, L#function)); \
    if (!function) return false; \

bool LoadBandFunction(void* dllHandle) {
    LoadFunction(dllHandle, TfLiteVersion);
    LoadFunction(dllHandle, TfLiteModelCreate);
    LoadFunction(dllHandle, TfLiteModelCreateFromFile);
    LoadFunction(dllHandle, TfLiteInterpreterOptionsCreate);
    LoadFunction(dllHandle, TfLiteInterpreterOptionsDelete);
    LoadFunction(dllHandle, TfLiteInterpreterCreate);
    LoadFunction(dllHandle, TfLiteInterpreterDelete);
    LoadFunction(dllHandle, TfLiteInterpreterRegisterModel);
    LoadFunction(dllHandle, TfLiteInterpreterInvokeSync);
    LoadFunction(dllHandle, TfLiteInterpreterInvokeAsync);
    LoadFunction(dllHandle, TFLiteInterpreterWait);
    LoadFunction(dllHandle, TfLiteInterpreterGetInputTensorCount);
    LoadFunction(dllHandle, TfLiteInterpreterGetOutputTensorCount);
    LoadFunction(dllHandle, TfLiteInterpreterAllocateInputTensor);
    LoadFunction(dllHandle, TfLiteInterpreterAllocateOutputTensor);
    LoadFunction(dllHandle, TfLiteTensorDeallocate);
    LoadFunction(dllHandle, TfLiteTensorType);
    LoadFunction(dllHandle, TfLiteTensorNumDims);
    LoadFunction(dllHandle, TfLiteTensorDim);
    LoadFunction(dllHandle, TfLiteTensorByteSize);
    LoadFunction(dllHandle, TfLiteTensorData);
    LoadFunction(dllHandle, TfLiteTensorName);
    LoadFunction(dllHandle, TfLiteTensorQuantizationParams);
    LoadFunction(dllHandle, TfLiteTensorCopyFromBuffer);
    LoadFunction(dllHandle, TfLiteTensorCopyToBuffer);
    return true;
}