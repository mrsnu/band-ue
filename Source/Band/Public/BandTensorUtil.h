#pragma once

#include "Engine/Texture.h"
#include "BandTensor.h"
#include <cstdint>

namespace BandTensorUtil {
TArray<BandTensor*> TensorsFromTArray(TArray<UBandTensor*> Tensors);

template <typename T>
void BGRA8ToRGBArray(const uint8_t* Src, T* Dst, size_t NumElements, T Mean,
                     T Std) {
  for (size_t i = 0, j = 0; i < NumElements; i++) {
    Dst[j++] = (static_cast<T>(Src[i * 4 + 2]) - Mean) / Std; // R
    Dst[j++] = (static_cast<T>(Src[i * 4 + 1]) - Mean) / Std; // G
    Dst[j++] = (static_cast<T>(Src[i * 4]) - Mean) / Std;     // B
  }
}

template <typename T>
void RGBA8ToRGBArray(const uint8_t* Src, T* Dst, size_t NumElements, T Mean,
                     T Std) {
  for (size_t i = 0, j = 0; i < NumElements; i++) {
    Dst[j++] = (static_cast<T>(Src[i * 4]) - Mean) / Std;     // R
    Dst[j++] = (static_cast<T>(Src[i * 4 + 1]) - Mean) / Std; // G
    Dst[j++] = (static_cast<T>(Src[i * 4 + 2]) - Mean) / Std; // B
  }
}

template <typename T>
void RGB8ToRGBArray(const uint8_t* Src, T* Dst, size_t NumElements, T Mean,
                    T Std) {
  for (size_t i = 0, j = 0; i < NumElements; i++) {
    Dst[j++] = (static_cast<T>(Src[i * 3]) - Mean) / Std;     // R
    Dst[j++] = (static_cast<T>(Src[i * 3 + 1]) - Mean) / Std; // G
    Dst[j++] = (static_cast<T>(Src[i * 3 + 2]) - Mean) / Std; // B
  }
}

template <typename T>
bool TextureToRGBArray(const void* Source, EPixelFormat PixelFormat, T* Dst,
                       size_t NumElements, T Mean, T Std) {
  switch (PixelFormat) {
    case PF_R8G8B8A8:
      RGBA8ToRGBArray<T>(reinterpret_cast<const uint8_t*>(Source), Dst,
                         NumElements, Mean, Std);
      break;
    case PF_B8G8R8A8:
      BGRA8ToRGBArray<T>(reinterpret_cast<const uint8_t*>(Source), Dst,
                         NumElements, Mean, Std);
      break;
    default:
      UE_LOG(LogBand, Log, TEXT("Not support pixel format %d"), PixelFormat);
      return false;
  }
  return true;
}

template <typename DST_T, typename SRC_T>
bool FromTArray(TArray<SRC_T> Elements, DST_T* Dst, int NumElements) {
  if (Elements.Num() > NumElements) {
    UE_LOG(LogBand, Error,
           TEXT(
             "Number of elements filling exceeds number of elements to fill (%d vs %d)"
           ), Elements.Num(), NumElements);
    return false;
  }
  for (int i = 0; i < Elements.Num(); i++) {
    Dst[i] = (static_cast<DST_T>(Elements[i]));
  }
  return true;
}
} // namespace BandTensorUtil
