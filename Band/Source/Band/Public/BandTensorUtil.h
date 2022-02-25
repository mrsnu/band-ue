#pragma once

#include "Engine/Texture.h"
#include <cstdint>

namespace BandTensorUtil
{
	template <typename T>
	void BGRA8ToRGBArray(const uint8_t* Src, T* Dst, size_t NumElements, T Mean, T Std)
	{
		for (size_t i = 0, j = 0; i < NumElements; i++)
		{
			Dst[j++] = (T)((Src[i * 4 + 2] - Mean) / Std); // R
			Dst[j++] = (T)((Src[i * 4 + 1] - Mean) / Std); // G
			Dst[j++] = (T)((Src[i * 4] - Mean) / Std); // B
		}
	}

	template <typename T>
	void RGBA8ToRGBArray(const uint8_t* Src, T* Dst, size_t NumElements, T Mean, T Std)
	{
		for (size_t i = 0, j = 0; i < NumElements; i++)
		{
			Dst[j++] = (T)((Src[i * 4] - Mean) / Std); // R
			Dst[j++] = (T)((Src[i * 4 + 1] - Mean) / Std); // G
			Dst[j++] = (T)((Src[i * 4 + 2] - Mean) / Std); // B
		}
	}

	template <typename T>
	bool TextureToRGBArray(FTextureSource& Source, T* Dst, T Mean, T Std)
	{
		uint8* SourceData = Source.LockMip(0);
		switch (Source.GetFormat())
		{
		case TSF_RGBA8:
			RGBA8ToRGBArray<T>(SourceData, Dst, Source.GetSizeX() * Source.GetSizeY(), Mean, Std);
			break;
		case TSF_BGRA8:
			BGRA8ToRGBArray<T>(SourceData, Dst, Source.GetSizeX() * Source.GetSizeY(), Mean, Std);
			break;
		default:
			Source.UnlockMip(0);
			return false;
		}
		Source.UnlockMip(0);
		return true;
	}
}