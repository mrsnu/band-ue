#pragma once

#include "BandLibrary/BandLibrary.h"

void LoadBandFunction(void* dllHandle);

typedef struct TfLiteModel TfLiteModel;
typedef const char* (*pTfLiteVersion)(void);
typedef TfLiteModel* (*pTfLiteModelCreate)(const void* /* model_data */, size_t /* model_size */);

pTfLiteVersion TfLiteVersion;
pTfLiteModelCreate TfLiteModelCreate;