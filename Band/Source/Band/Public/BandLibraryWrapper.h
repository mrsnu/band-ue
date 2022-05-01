#pragma once

#include "BandLibrary.h"

namespace Band
{
	typedef struct TfLiteModel TfLiteModel;
	typedef struct TfLiteInterpreterOptions TfLiteInterpreterOptions;
	typedef struct TfLiteInterpreter TfLiteInterpreter;
	typedef void (*TfLiteErrorReporter)(void* user_data, const char* format, va_list args);
	/*
		function pointer definitions (corresponds to c_api.h)
	*/
	typedef const char* (*pTfLiteVersion)();
	typedef TfLiteModel* (*pTfLiteModelCreate)(
		const void* model_data,
		size_t model_size);
	typedef TfLiteModel* (*pTfLiteModelCreateFromFile)(
		const char* model_path);
	typedef void (*pTfLiteModelDelete)(
		TfLiteModel* model);
	/* Interpreter helper methods */
	typedef TfLiteInterpreterOptions* (*pTfLiteInterpreterOptionsCreate)();
	typedef void (*pTfLiteInterpreterOptionsDelete)(
		TfLiteInterpreterOptions* options);
	typedef void (*pTfLiteInterpreterOptionsSetErrorReporter)(
		TfLiteInterpreterOptions* options,
		void (*reporter)(void* user_data,
			const char* format, va_list args),
		void* user_data);
	typedef void (*pTfLiteInterpreterOptionsSetOnInvokeEnd)(
		TfLiteInterpreterOptions* options,
		void (*on_invoke_end)(void* user_data, int job_id, TfLiteStatus status),
		void* user_data);
	typedef TfLiteStatus (*pTfLiteInterpreterOptionsSetConfigPath)(
		TfLiteInterpreterOptions* options,
		const char* config_path);
	typedef TfLiteStatus (*pTfLiteInterpreterOptionsSetConfigFile)(
		TfLiteInterpreterOptions* options,
		const void* config_data, size_t config_size);
	typedef TfLiteInterpreter* (*pTfLiteInterpreterCreate)(
		const TfLiteInterpreterOptions* optional_options);
	typedef void (*pTfLiteInterpreterDelete)(
		TfLiteInterpreter* interpreter);
	typedef int32_t (*pTfLiteInterpreterRegisterModel)(
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
	typedef TfLiteStatus (*pTfLiteInterpreterWait)(
		TfLiteInterpreter* interpreter,
		int job_id,
		TfLiteTensor** outputs);
	typedef int32_t (*pTfLiteInterpreterGetInputTensorCount)(
		const TfLiteInterpreter* interpreter,
		int32_t model_id);
	typedef int32_t (*pTfLiteInterpreterGetOutputTensorCount)(
		const TfLiteInterpreter* interpreter,
		int32_t model_id);
	typedef TfLiteTensor* (*pTfLiteInterpreterAllocateInputTensor)(
		const TfLiteInterpreter* interpreter,
		int32_t model_id, int32_t input_index);
	typedef TfLiteTensor* (*pTfLiteInterpreterAllocateOutputTensor)(
		const TfLiteInterpreter* interpreter,
		int32_t model_id,
		int32_t output_index);
	/* Tensor helper methods */
	typedef void (*pTfLiteTensorDeallocate)(
		TfLiteTensor* tensor);
	typedef TfLiteType (*pTfLiteTensorType)(
		const TfLiteTensor* tensor);
	typedef int32_t (*pTfLiteTensorNumDims)(
		const TfLiteTensor* tensor);
	typedef int32_t (*pTfLiteTensorDim)(
		const TfLiteTensor* tensor,
		int32_t dim_index);
	typedef size_t (*pTfLiteTensorByteSize)(
		const TfLiteTensor* tensor);
	typedef void* (*pTfLiteTensorData)(
		const TfLiteTensor* tensor);
	typedef const char* (*pTfLiteTensorName)(
		const TfLiteTensor* tensor);
	typedef TfLiteQuantizationParams (*pTfLiteTensorQuantizationParams)(
		const TfLiteTensor* tensor);
	typedef TfLiteStatus (*pTfLiteTensorCopyFromBuffer)(
		TfLiteTensor* tensor,
		const void* input_data,
		size_t input_data_size);
	typedef TfLiteStatus (*pTfLiteTensorCopyToBuffer)(
		const TfLiteTensor* output_tensor,
		void* output_data,
		size_t output_data_size);

	/*
		function pointer definitions (corresponds to c_api_experimental.h)
	*/
	typedef void (*pTfLiteModelAddBuiltinOp)(
		TfLiteModel* model, TfLiteBuiltinOperator op,
		const TfLiteRegistration* registration, int32_t min_version,
		int32_t max_version);
	typedef void (*pTfLiteModelOptionsAddCustomOp)(
		TfLiteModel* model, const char* name,
		const TfLiteRegistration* registration, int32_t min_version,
		int32_t max_version);
	/*
		global handles
	*/
	pTfLiteVersion TfLiteVersion = nullptr;
	pTfLiteModelCreate TfLiteModelCreate = nullptr;
	pTfLiteModelCreateFromFile TfLiteModelCreateFromFile = nullptr;
	pTfLiteModelDelete TfLiteModelDelete = nullptr;
	pTfLiteInterpreterOptionsCreate TfLiteInterpreterOptionsCreate = nullptr;
	pTfLiteInterpreterOptionsDelete TfLiteInterpreterOptionsDelete = nullptr;
	pTfLiteInterpreterOptionsSetOnInvokeEnd TfLiteInterpreterOptionsSetOnInvokeEnd = nullptr;
	pTfLiteInterpreterOptionsSetConfigPath TfLiteInterpreterOptionsSetConfigPath = nullptr;
	pTfLiteInterpreterOptionsSetConfigFile TfLiteInterpreterOptionsSetConfigFile = nullptr;
	pTfLiteInterpreterOptionsSetErrorReporter TfLiteInterpreterOptionsSetErrorReporter = nullptr;
	pTfLiteInterpreterCreate TfLiteInterpreterCreate = nullptr;
	pTfLiteInterpreterDelete TfLiteInterpreterDelete = nullptr;
	pTfLiteInterpreterRegisterModel TfLiteInterpreterRegisterModel = nullptr;
	pTfLiteInterpreterInvokeSync TfLiteInterpreterInvokeSync = nullptr;
	pTfLiteInterpreterInvokeAsync TfLiteInterpreterInvokeAsync = nullptr;
	pTfLiteInterpreterWait TfLiteInterpreterWait = nullptr;
	pTfLiteInterpreterGetInputTensorCount TfLiteInterpreterGetInputTensorCount = nullptr;
	pTfLiteInterpreterGetOutputTensorCount TfLiteInterpreterGetOutputTensorCount = nullptr;
	pTfLiteInterpreterAllocateInputTensor TfLiteInterpreterAllocateInputTensor = nullptr;
	pTfLiteInterpreterAllocateOutputTensor TfLiteInterpreterAllocateOutputTensor = nullptr;
	pTfLiteTensorDeallocate TfLiteTensorDeallocate = nullptr;
	pTfLiteTensorType TfLiteTensorType = nullptr;
	pTfLiteTensorNumDims TfLiteTensorNumDims = nullptr;
	pTfLiteTensorDim TfLiteTensorDim = nullptr;
	pTfLiteTensorByteSize TfLiteTensorByteSize = nullptr;
	pTfLiteTensorData TfLiteTensorData = nullptr;
	pTfLiteTensorName TfLiteTensorName = nullptr;
	pTfLiteTensorQuantizationParams TfLiteTensorQuantizationParams = nullptr;
	pTfLiteTensorCopyFromBuffer TfLiteTensorCopyFromBuffer = nullptr;
	pTfLiteTensorCopyToBuffer TfLiteTensorCopyToBuffer = nullptr;
} // namespace Band
