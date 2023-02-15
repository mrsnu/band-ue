#ifndef BAND_C_C_API_H_
#define BAND_C_C_API_H_

#include <stdarg.h>
#include <stdint.h>

#include "c_api_types.h"
#include "common.h"

#ifdef SWIG
#define BAND_CAPI_EXPORT
#else
// TODO: Add BAND_CAPI_EXPORT flag to support external symbols to dll (windows
// platform)
#if defined(_WIN32)
#ifdef BAND_COMPILE_LIBRARY
#define BAND_CAPI_EXPORT __declspec(dllexport)
#else
#define BAND_CAPI_EXPORT __declspec(dllimport)
#endif  // BAND_COMPILE_LIBRARY
#else
#define BAND_CAPI_EXPORT __attribute__((visibility("default")))
#endif  // _WIN32
#endif  // SWIG

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Forward decl of internal types - details are in `c_api_types.h`
using BandConfigBuilder = struct BandConfigBuilder;
using BandConfig = struct BandConfig;
using BandModel = struct BandModel;
using BandTensor = struct BandTensor;
using BandEngine = struct BandEngine;
using BandRequestHandle = int;

/* config builder */
BAND_CAPI_EXPORT extern BandConfigBuilder* BandConfigBuilderCreate();
BAND_CAPI_EXPORT extern void BandAddConfig(BandConfigBuilder* b, int field,
                                           int count, ...);
BAND_CAPI_EXPORT extern void BandConfigBuilderDelete(BandConfigBuilder* b);

/* config */
BAND_CAPI_EXPORT extern BandConfig* BandConfigCreate(BandConfigBuilder* b);
BAND_CAPI_EXPORT extern void BandConfigDelete(BandConfig* config);

/* model */
BAND_CAPI_EXPORT extern BandModel* BandModelCreate();
BAND_CAPI_EXPORT extern void BandModelDelete(BandModel* model);
BAND_CAPI_EXPORT extern BandStatus BandModelAddFromBuffer(
    BandModel* model, BandBackendType backend_type, const void* model_data,
    size_t model_size);
BAND_CAPI_EXPORT extern BandStatus BandModelAddFromFile(
    BandModel* model, BandBackendType backend_type, const char* model_path);

/* tensor */
// Band intetionally `only` expose getters to ensure
BAND_CAPI_EXPORT extern void BandTensorDelete(BandTensor* tensor);
BAND_CAPI_EXPORT extern BandType BandTensorGetType(BandTensor* tensor);
BAND_CAPI_EXPORT extern void* BandTensorGetData(BandTensor* tensor);
BAND_CAPI_EXPORT extern size_t BandTensorGetNumDims(BandTensor* tensor);
BAND_CAPI_EXPORT extern const int* BandTensorGetDims(BandTensor* tensor);
BAND_CAPI_EXPORT extern size_t BandTensorGetBytes(BandTensor* tensor);
BAND_CAPI_EXPORT extern const char* BandTensorGetName(BandTensor* tensor);
BAND_CAPI_EXPORT extern BandQuantization BandTensorGetQuantization(
    BandTensor* tensor);

/* engine */
// TODO: Error reporter
BAND_CAPI_EXPORT extern BandEngine* BandEngineCreate(BandConfig* config);
BAND_CAPI_EXPORT extern void BandEngineDelete(BandEngine* engine);
BAND_CAPI_EXPORT extern BandStatus BandEngineRegisterModel(BandEngine* engine,
  BandModel* model);
BAND_CAPI_EXPORT extern int BandEngineGetNumInputTensors(BandEngine* engine,
  BandModel* model);
BAND_CAPI_EXPORT extern int BandEngineGetNumOutputTensors(BandEngine* engine,
  BandModel* model);

BAND_CAPI_EXPORT extern int BandEngineGetNumWorkers(BandEngine* engine);
BAND_CAPI_EXPORT extern BandDeviceFlags BandEngineGetWorkerDevice(
    BandEngine* engine, int worker_id);

// Create a input tensor for given model's n'th index
BAND_CAPI_EXPORT
extern BandTensor* BandEngineCreateInputTensor(BandEngine* engine,
                                               BandModel* model, size_t index);
// Create a output tensor for given model's n'th index
BAND_CAPI_EXPORT extern BandTensor* BandEngineCreateOutputTensor(
    BandEngine* engine, BandModel* model, size_t index);
BAND_CAPI_EXPORT extern BandStatus BandEngineRequestSync(
    BandEngine* engine, BandModel* model, BandTensor** input_tensors,
    BandTensor** output_tensors);
BAND_CAPI_EXPORT extern BandRequestHandle BandEngineRequestAsync(
    BandEngine* engine, BandModel* model, BandTensor** input_tensors);
BAND_CAPI_EXPORT extern BandStatus BandEngineRequestSyncOnWorker(
    BandEngine* engine, BandModel* model, int target_worker,
    BandTensor** input_tensors, BandTensor** output_tensors);
BAND_CAPI_EXPORT extern BandRequestHandle BandEngineRequestAsyncOnWorker(
    BandEngine* engine, BandModel* model, int target_worker,
    BandTensor** input_tensors);
BAND_CAPI_EXPORT extern BandStatus BandEngineWait(BandEngine* engine,
                                                  BandRequestHandle handle,
                                                  BandTensor** output_tensors,
                                                  size_t num_outputs);
BAND_CAPI_EXPORT extern void BandEngineSetOnEndRequest(
    BandEngine* engine,
    void (*on_end_invoke)(void* user_data, int job_id, BandStatus status),
    void* user_data);

using PFN_BandConfigBuilderCreate = BandConfigBuilder* (*)();
using PFN_BandAddConfig = void(*)(BandConfigBuilder*, int, int, ...);
using PFN_BandConfigBuilderDelete = void(*)(BandConfigBuilder*);
using PFN_BandConfigCreate = BandConfig* (*)(BandConfigBuilder*);
using PFN_BandConfigDelete = void(*)(BandConfig*);
using PFN_BandModelCreate = BandModel* (*)();
using PFN_BandModelDelete = void(*)(BandModel*);
using PFN_BandModelAddFromBuffer = BandStatus(*)(BandModel*, BandBackendType,
                                                 const void*, size_t);
using PFN_BandModelAddFromFile = BandStatus(*)(BandModel*, BandBackendType,
                                               const char*);
using PFN_BandTensorDelete = void(*)(BandTensor*);
using PFN_BandTensorGetType = BandType(*)(BandTensor*);
using PFN_BandTensorGetData = void* (*)(BandTensor*);
using PFN_BandTensorGetNumDims = size_t(*)(BandTensor*);
using PFN_BandTensorGetDims = const int* (*)(BandTensor*);
using PFN_BandTensorGetBytes = size_t(*)(BandTensor*);
using PFN_BandTensorGetName = const char* (*)(BandTensor*);
using PFN_BandTensorGetQuantization = BandQuantization(*)(BandTensor*);
using PFN_BandEngineCreate = BandEngine* (*)(BandConfig*);
using PFN_BandEngineDelete = void(*)(BandEngine*);
using PFN_BandEngineRegisterModel = BandStatus(*)(BandEngine*, BandModel*);
using PFN_BandEngineGetNumInputTensors = int(*)(BandEngine*, BandModel*);
using PFN_BandEngineGetNumOutputTensors = int(*)(BandEngine*, BandModel*);
using PFN_BandEngineGetNumWorkers = int(*)(BandEngine*);
using PFN_BandEngineGetWorkerDevice = BandDeviceFlags(*)(BandEngine*, int);
using PFN_BandEngineCreateInputTensor = BandTensor* (*)(BandEngine*, BandModel*,
  size_t);
using PFN_BandEngineCreateOutputTensor = BandTensor* (*)(
    BandEngine*, BandModel*,
    size_t);
using PFN_BandEngineRequestSync = BandStatus(*)(BandEngine*, BandModel*,
                                                BandTensor**, BandTensor**);
using PFN_BandEngineRequestAsync = BandRequestHandle(*)(BandEngine*, BandModel*,
  BandTensor**);
using PFN_BandEngineRequestSyncOnWorker = BandStatus(*)(BandEngine*, BandModel*,
  int, BandTensor**,
  BandTensor**);
using PFN_BandEngineRequestAsyncOnWorker = BandRequestHandle(*)(BandEngine*,
  BandModel*, int,
  BandTensor**);
using PFN_BandEngineWait = BandStatus(*)(BandEngine*, BandRequestHandle,
                                         BandTensor**, size_t);
using PFN_BandEngineSetOnEndRequest = void(*)(BandEngine*,
                                              void (*)(void*, int, BandStatus),
                                              void*);

#ifdef __cplusplus
} // extern "C"
#endif  // __cplusplus
#endif
