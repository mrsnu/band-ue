/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#pragma once

#include <memory>

#include "Core.h"
#include "HAL/PlatformTime.h"

#include "FrameBuffer.h"
#include "AndroidCameraFrame.h"

namespace band {
constexpr int kRgbaPixelBytes = 4, kRgbPixelBytes = 3, kGrayPixelBytes = 1;
// Default stride value for creating frame buffer from raw buffer. When using
// this default value, the default row stride and pixel stride values will be
// applied. e.g. for RGB image, row_stride = width * kRgbPixelBytes,
// pixel_stride = kRgbPixelBytes.
constexpr FrameBuffer::Stride kDefaultStride = {0, 0};

// Miscellaneous Methods
// -----------------------------------------------------------------

// Returns the frame buffer size in bytes based on the input format and
// dimensions. GRAY, YV12/YV21 are in the planar formats, NV12/NV21 are in the
// semi-planar formats with the interleaved UV planes. RGB/RGBA are in the
// interleaved format.
int GetFrameBufferByteSize(FrameBuffer::Dimension dimension,
                           FrameBuffer::Format format);

// Returns pixel stride info for kGRAY, kRGB, kRGBA formats.
int GetPixelStrides(FrameBuffer::Format format);

// Returns the biplanar UV raw buffer for NV12/NV21 frame buffer.
const uint8* GetUvRawBuffer(
    const FrameBuffer& buffer);

// Returns U or V plane dimension with the given buffer `dimension` and
// `format`. Only supports NV12/NV21/YV12/YV21 formats. Returns
// InvalidArgumentError if 'dimension' is invalid or 'format' is other than the
// supported formats. This method assums the UV plane share the same dimension,
// especially for the YV12 / YV21 formats.
FrameBuffer::Dimension GetUvPlaneDimension(
    FrameBuffer::Dimension dimension, FrameBuffer::Format format);

// Returns crop dimension based on crop start and end points.
FrameBuffer::Dimension GetCropDimension(int x0, int x1, int y0, int y1);

// Validation Methods
// -----------------------------------------------------------------

// Validates that the given buffer has the correct metadata. Returns error
// state when any buffer has missing stride info.
bool ValidateBufferPlaneMetadata(const FrameBuffer& buffer);

// Validates that the given buffer has the correct format for its configuration.
bool ValidateBufferFormat(const FrameBuffer& buffer);

// Validates that the given buffers have the correct format for their
// configuration.
bool ValidateBufferFormats(const FrameBuffer& buffer1,
                           const FrameBuffer& buffer2);

// Validates the given inputs for resizing `buffer`.
bool ValidateResizeBufferInputs(const FrameBuffer& buffer,
                                const FrameBuffer& output_buffer);

// Validates the given inputs for rotating `buffer`.
bool ValidateRotateBufferInputs(const FrameBuffer& buffer,
                                const FrameBuffer& output_buffer,
                                int angle_deg);

// Validates the given inputs for cropping `buffer`.
//
// (x0, y0) represents the top-left point of the buffer.
// (x1, y1) represents the bottom-right point of the buffer.
bool ValidateCropBufferInputs(const FrameBuffer& buffer,
                              const FrameBuffer& output_buffer, int x0,
                              int y0, int x1, int y1);

// Validates the given inputs for flipping `buffer` horizontally or vertically.
bool ValidateFlipBufferInputs(const FrameBuffer& buffer,
                              const FrameBuffer& output_buffer);

// Validates that `from_format` can be converted to `to_format`.
//
// The given formats must not be equal.
bool ValidateConvertFormats(FrameBuffer::Format from_format,
                            FrameBuffer::Format to_format);

// Creation Methods
// -----------------------------------------------------------------

// Creates a FrameBuffer from raw RGBA buffer and passing arguments.
BAND_API std::unique_ptr<FrameBuffer> CreateFromRgbaRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation = FrameBuffer::Orientation::kTopLeft,
    uint64 timestamp = FPlatformTime::Cycles64(),
    FrameBuffer::Stride stride = kDefaultStride);

// Creates a FrameBuffer from raw RGB buffer and passing arguments.
BAND_API std::unique_ptr<FrameBuffer> CreateFromRgbRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation = FrameBuffer::Orientation::kTopLeft,
    uint64 timestamp = FPlatformTime::Cycles64(),
    FrameBuffer::Stride stride = kDefaultStride);

// Creates a FrameBuffer from raw grayscale buffer and passing arguments.
BAND_API std::unique_ptr<FrameBuffer> CreateFromGrayRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation = FrameBuffer::Orientation::kTopLeft,
    uint64 timestamp = FPlatformTime::Cycles64(),
    FrameBuffer::Stride stride = kDefaultStride);

// Creates a FrameBuffer from raw YUV buffer and passing arguments.
BAND_API std::unique_ptr<FrameBuffer> CreateFromYuvRawBuffer(
    const uint8* y_plane, const uint8* u_plane, const uint8* v_plane,
    FrameBuffer::Format format, FrameBuffer::Dimension dimension,
    int row_stride_y, int row_stride_uv, int pixel_stride_uv,
    FrameBuffer::Orientation orientation = FrameBuffer::Orientation::kTopLeft,
    uint64 timestamp = FPlatformTime::Cycles64());

// Creates an instance of FrameBuffer from raw buffer and passing arguments.
BAND_API std::unique_ptr<FrameBuffer> CreateFromRawBuffer(
    const uint8* buffer, FrameBuffer::Dimension dimension,
    FrameBuffer::Format target_format,
    FrameBuffer::Orientation orientation = FrameBuffer::Orientation::kTopLeft,
    uint64 timestamp = FPlatformTime::Cycles64());

BAND_API std::unique_ptr<FrameBuffer> CreateFromAndroidCameraFrame(
    const UAndroidCameraFrame& camera_frame);
} // namespace Band
