#pragma once

#include "BandLibrary/BandLibrary.h"

bool LoadBandFunction(void *dllHandle);

typedef struct TfLiteModel TfLiteModel;
typedef const char *(*pTfLiteVersion)();
typedef TfLiteModel *(*pTfLiteModelCreate)(
    const void *model_data, 
    size_t model_size);
typedef TfLiteModel *(*pTfLiteModelCreateFromFile)(
    const char *model_path);
typedef void (*pTfLiteModelDelete)(
    TfLiteModel *model);

typedef struct TfLiteInterpreterOptions TfLiteInterpreterOptions;
typedef TfLiteInterpreterOptions *(*pTfLiteInterpreterOptionsCreate)();
typedef void (*pTfLiteInterpreterOptionsDelete)(
    TfLiteInterpreterOptions *options);
typedef void (*pTfLiteInterpreterOptionsSetErrorReporter)(
    TfLiteInterpreterOptions *options, 
    void (*reporter)(void *user_data, 
    const char *format, va_list args), 
    void *user_data);
typedef TfLiteStatus (*pTfLiteInterpreterOptionsSetConfigPath)(
    TfLiteInterpreterOptions *options,
    const char *config_path);
typedef TfLiteStatus(*pTfLiteInterpreterOptionsSetConfigFile)(
    TfLiteInterpreterOptions* options,
    const void* config_data, size_t config_size);

typedef struct TfLiteInterpreter TfLiteInterpreter;
typedef TfLiteInterpreter* (*pTfLiteInterpreterCreate)(
    const TfLiteInterpreterOptions* optional_options);
typedef void (*pTfLiteInterpreterDelete)(
    TfLiteInterpreter* interpreter);
typedef int32_t(*pTfLiteInterpreterRegisterModel)(
    TfLiteInterpreter* interpreter, 
    TfLiteModel* model);
typedef void (*pTfLiteInterpreterInvokeSync)(
    TfLiteInterpreter* interpreter, 
    int32_t model_id, 
    TfLiteTensor** inputs, 
    TfLiteTensor** outputs);
typedef int (*pTfLiteInterpreterInvokeAsync)(
    TfLiteInterpreter* interpreter, 
    int32_t model_id, 
    TfLiteTensor** inputs);
typedef TfLiteStatus(*pTFLiteInterpreterWait)(
    TfLiteInterpreter* interpreter, 
    int job_id, 
    TfLiteTensor** outputs);
typedef int32_t(*pTfLiteInterpreterGetInputTensorCount)(
    const TfLiteInterpreter* interpreter, 
    int32_t model_id);
typedef int32_t(*pTfLiteInterpreterGetOutputTensorCount)(
    const TfLiteInterpreter* interpreter, 
    int32_t model_id);
typedef TfLiteTensor* (*pTfLiteInterpreterAllocateInputTensor)(
    const TfLiteInterpreter* interpreter, 
    int32_t model_id, int32_t input_index);
typedef TfLiteTensor* (*pTfLiteInterpreterAllocateOutputTensor)(
    const TfLiteInterpreter* interpreter, 
    int32_t model_id, 
    int32_t output_index);
typedef void (*pTfLiteTensorDeallocate)(
    TfLiteTensor* tensor);
typedef TfLiteType(*pTfLiteTensorType)(
    const TfLiteTensor* tensor);
typedef int32_t(*pTfLiteTensorNumDims)(
    const TfLiteTensor* tensor);
typedef int32_t(*pTfLiteTensorDim)(
    const TfLiteTensor* tensor,
    int32_t dim_index);
typedef size_t(*pTfLiteTensorByteSize)(
    const TfLiteTensor* tensor);
typedef void* (*pTfLiteTensorData)(
    const TfLiteTensor* tensor);
typedef const char* (*pTfLiteTensorName)(
    const TfLiteTensor* tensor);
typedef TfLiteQuantizationParams(*pTfLiteTensorQuantizationParams)(
    const TfLiteTensor* tensor);
typedef TfLiteStatus(*pTfLiteTensorCopyFromBuffer)(
    TfLiteTensor* tensor, 
    const void* input_data, 
    size_t input_data_size);
typedef TfLiteStatus (*pTfLiteTensorCopyToBuffer)(
    const TfLiteTensor* output_tensor, 
    void* output_data,
    size_t output_data_size);

pTfLiteVersion TfLiteVersion;
pTfLiteModelCreate TfLiteModelCreate;
pTfLiteModelCreateFromFile TfLiteModelCreateFromFile;
pTfLiteModelDelete TfLiteModelDelete;
pTfLiteInterpreterOptionsCreate TfLiteInterpreterOptionsCreate;
pTfLiteInterpreterOptionsDelete TfLiteInterpreterOptionsDelete;
pTfLiteInterpreterCreate TfLiteInterpreterCreate;
pTfLiteInterpreterDelete TfLiteInterpreterDelete;
pTfLiteInterpreterRegisterModel TfLiteInterpreterRegisterModel;
pTfLiteInterpreterInvokeSync TfLiteInterpreterInvokeSync;
pTfLiteInterpreterInvokeAsync TfLiteInterpreterInvokeAsync;
pTFLiteInterpreterWait TFLiteInterpreterWait;
pTfLiteInterpreterGetInputTensorCount TfLiteInterpreterGetInputTensorCount;
pTfLiteInterpreterGetOutputTensorCount TfLiteInterpreterGetOutputTensorCount;
pTfLiteInterpreterAllocateInputTensor TfLiteInterpreterAllocateInputTensor;
pTfLiteInterpreterAllocateOutputTensor TfLiteInterpreterAllocateOutputTensor;
pTfLiteTensorDeallocate TfLiteTensorDeallocate;
pTfLiteTensorType TfLiteTensorType;
pTfLiteTensorNumDims TfLiteTensorNumDims;
pTfLiteTensorDim TfLiteTensorDim;
pTfLiteTensorByteSize TfLiteTensorByteSize;
pTfLiteTensorData TfLiteTensorData;
pTfLiteTensorName TfLiteTensorName;
pTfLiteTensorQuantizationParams TfLiteTensorQuantizationParams;
pTfLiteTensorCopyFromBuffer TfLiteTensorCopyFromBuffer;
pTfLiteTensorCopyToBuffer TfLiteTensorCopyToBuffer;