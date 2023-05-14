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

#include "Band.h"
#include "ImageUtil/Common.h"
#include "ImageUtil/FrameBuffer.h"
#include "ImageUtil/FrameBufferCommonUtils.h"

#include <string>
#include <vector>

namespace band {
namespace {
constexpr int kRgbaChannels = 4;
constexpr int kRgbChannels = 3;
constexpr int kGrayChannel = 1;

// Creates a FrameBuffer from one plane raw NV21/NV12 buffer and passing
// arguments.
std::unique_ptr<FrameBuffer> CreateFromOnePlaneNVRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Format format, FrameBuffer::Orientation orientation,
    const uint64 timestamp) {
  FrameBuffer::Plane input_plane = {/*buffer=*/input,
                                               /*stride=*/
                                               {dimension.width, kGrayChannel}};
  return FrameBuffer::Create({input_plane}, dimension, format, orientation,
                             timestamp);
}

// Indicates whether the given buffers have the same dimensions.
bool AreBufferDimsEqual(const FrameBuffer& buffer1,
                        const FrameBuffer& buffer2) {
  return buffer1.dimension() == buffer2.dimension();
}

// Indicates whether the given buffers formats are compatible. Same formats are
// compatible and all YUV family formats (e.g. NV21, NV12, YV12, YV21, etc) are
// compatible.
bool AreBufferFormatsCompatible(const FrameBuffer& buffer1,
                                const FrameBuffer& buffer2) {
  switch (buffer1.format()) {
    case FrameBuffer::Format::kRGBA:
    case FrameBuffer::Format::kRGB:
      return (buffer2.format() == FrameBuffer::Format::kRGBA ||
              buffer2.format() == FrameBuffer::Format::kRGB);
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kYV12:
    case FrameBuffer::Format::kYV21:
      return (buffer2.format() == FrameBuffer::Format::kNV12 ||
              buffer2.format() == FrameBuffer::Format::kNV21 ||
              buffer2.format() == FrameBuffer::Format::kYV12 ||
              buffer2.format() == FrameBuffer::Format::kYV21);
    case FrameBuffer::Format::kGRAY:
    default:
      return buffer1.format() == buffer2.format();
  }
}
} // namespace

// Miscellaneous Methods
// -----------------------------------------------------------------
int GetFrameBufferByteSize(FrameBuffer::Dimension dimension,
                           FrameBuffer::Format format) {
  switch (format) {
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kYV12:
    case FrameBuffer::Format::kYV21:
      return /*y plane*/ dimension.Size() +
                         /*uv plane*/ (dimension.width + 1) / 2 * (
                           dimension.height + 1) /
                         2 * 2;
    case FrameBuffer::Format::kRGB:
      return dimension.Size() * kRgbPixelBytes;
    case FrameBuffer::Format::kRGBA:
      return dimension.Size() * kRgbaPixelBytes;
    case FrameBuffer::Format::kGRAY:
      return dimension.Size();
    default:
      return 0;
  }
}

int GetPixelStrides(FrameBuffer::Format format) {
  switch (format) {
    case FrameBuffer::Format::kGRAY:
      return kGrayPixelBytes;
    case FrameBuffer::Format::kRGB:
      return kRgbPixelBytes;
    case FrameBuffer::Format::kRGBA:
      return kRgbaPixelBytes;
    default:
      UE_LOG(LogBand, Display,
             TEXT("GetPixelStrides does not support format: %i"), format);
      return -1;
  }
}

const uint8* GetUvRawBuffer(const FrameBuffer& buffer) {
  if (buffer.format() != FrameBuffer::Format::kNV12 &&
      buffer.format() != FrameBuffer::Format::kNV21) {
    UE_LOG(LogBand, Display,
           TEXT(
             "Only support getting biplanar UV buffer from NV12/NV21 frame buffer"
           ));
    return nullptr;
  }

  FrameBuffer::YuvData yuv_data =
      FrameBuffer::GetYuvDataFromFrameBuffer(buffer);
  if (yuv_data.u_buffer == nullptr && yuv_data.v_buffer == nullptr) {
    return nullptr;
  }
  const uint8* uv_buffer = buffer.format() == FrameBuffer::Format::kNV12
                             ? yuv_data.u_buffer
                             : yuv_data.v_buffer;
  return uv_buffer;
}

FrameBuffer::Dimension GetUvPlaneDimension(
    FrameBuffer::Dimension dimension, FrameBuffer::Format format) {
  if (dimension.width <= 0 || dimension.height <= 0) {
    UE_LOG(LogBand, Display, TEXT("Invalid input dimension: {%d, %d}"),
           dimension.width,
           dimension.height);
    return {};
  }
  switch (format) {
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kYV12:
    case FrameBuffer::Format::kYV21:
      return FrameBuffer::Dimension{(dimension.width + 1) / 2,
                                    (dimension.height + 1) / 2};
    default:
      UE_LOG(LogBand, Display, TEXT("Input format is not YUV-like: %i"),
             format);
      return {};
  }
}

FrameBuffer::Dimension GetCropDimension(int x0, int x1, int y0, int y1) {
  return {x1 - x0 + 1, y1 - y0 + 1};
}

// Validation Methods
// -----------------------------------------------------------------

bool ValidateBufferPlaneMetadata(const FrameBuffer& buffer) {
  if (buffer.plane_count() < 1) {
    UE_LOG(LogBand, Display, TEXT("There must be at least 1 plane specified"));
    return false;
  }

  for (int i = 0; i < buffer.plane_count(); i++) {
    if (buffer.plane(i).stride.row_stride_bytes == 0 ||
        buffer.plane(i).stride.pixel_stride_bytes == 0) {
      UE_LOG(LogBand, Display, TEXT("Invalid stride information"));
      return false;
    }
  }

  return true;
}

bool ValidateBufferFormat(const FrameBuffer& buffer) {
  switch (buffer.format()) {
    case FrameBuffer::Format::kGRAY:
    case FrameBuffer::Format::kRGB:
    case FrameBuffer::Format::kRGBA:
      if (buffer.plane_count() == 1) {
        return true;
      }
      UE_LOG(LogBand, Display,
             TEXT("Plane count must be 1 for grayscale and RGB[a] buffers"));
      return false;
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kYV21:
    case FrameBuffer::Format::kYV12:
      return true;
    default:
      UE_LOG(LogBand, Display, TEXT("Unsupported buffer format: %i"),
             buffer.format());
      return false;
  }
}

bool ValidateBufferFormats(const FrameBuffer& buffer1,
                           const FrameBuffer& buffer2) {
  RETURN_IF_ERROR(ValidateBufferFormat(buffer1));
  RETURN_IF_ERROR(ValidateBufferFormat(buffer2));
  return true;
}

bool ValidateResizeBufferInputs(const FrameBuffer& buffer,
                                const FrameBuffer& output_buffer) {
  bool valid_format = false;
  switch (buffer.format()) {
    case FrameBuffer::Format::kGRAY:
    case FrameBuffer::Format::kRGB:
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kYV12:
    case FrameBuffer::Format::kYV21:
      valid_format = (buffer.format() == output_buffer.format());
      break;
    case FrameBuffer::Format::kRGBA:
      valid_format = (output_buffer.format() == FrameBuffer::Format::kRGBA ||
                      output_buffer.format() == FrameBuffer::Format::kRGB);
      break;
    default:
      UE_LOG(LogBand, Display, TEXT("Unsupported buffer format: %i"),
             buffer.format());
      return false;
  }
  if (!valid_format) {
    UE_LOG(LogBand, Display,
           TEXT("Input and output buffer formats must match"));
    return false;
  }
  return ValidateBufferFormats(buffer, output_buffer);
}

bool ValidateRotateBufferInputs(const FrameBuffer& buffer,
                                const FrameBuffer& output_buffer,
                                int angle_deg) {
  if (!AreBufferFormatsCompatible(buffer, output_buffer)) {
    UE_LOG(LogBand, Display,
           TEXT("Input and output buffer formats must match"));
    return false;
  }

  const bool is_dimension_change = (angle_deg / 90) % 2 == 1;
  const bool are_dimensions_rotated =
      (buffer.dimension().width == output_buffer.dimension().height) &&
      (buffer.dimension().height == output_buffer.dimension().width);
  const bool are_dimensions_equal =
      buffer.dimension() == output_buffer.dimension();

  if (angle_deg >= 360 || angle_deg <= 0 || angle_deg % 90 != 0) {
    UE_LOG(LogBand, Display,
           TEXT(
             "Rotation angle must be between 0 and 360, in multiples of 90 degrees"
           ));
    return false;
  } else if ((is_dimension_change && !are_dimensions_rotated) ||
             (!is_dimension_change && !are_dimensions_equal)) {
    UE_LOG(LogBand, Display,
           TEXT("Output buffer has invalid dimensions for rotation"));
    return false;
  }
  return true;
}

bool ValidateCropBufferInputs(const FrameBuffer& buffer,
                              const FrameBuffer& output_buffer, int x0,
                              int y0, int x1, int y1) {
  if (!AreBufferFormatsCompatible(buffer, output_buffer)) {
    UE_LOG(LogBand, Display,
           TEXT("Input and output buffer formats must match"));
    return false;
  }

  bool is_buffer_size_valid =
      ((x1 < buffer.dimension().width) && y1 < buffer.dimension().height);
  bool are_points_valid = (x0 >= 0) && (y0 >= 0) && (x1 >= x0) && (y1 >= y0);

  if (!is_buffer_size_valid || !are_points_valid) {
    UE_LOG(LogBand, Display, TEXT("Invalid crop coordinates"));
    return false;
  }
  return true;
}

bool ValidateFlipBufferInputs(const FrameBuffer& buffer,
                              const FrameBuffer& output_buffer) {
  if (!AreBufferFormatsCompatible(buffer, output_buffer)) {
    UE_LOG(LogBand, Display,
           TEXT("Input and output buffer formats must match"));
    return false;
  }
  return AreBufferDimsEqual(buffer, output_buffer);
}

bool ValidateConvertFormats(FrameBuffer::Format from_format,
                            FrameBuffer::Format to_format) {
  if (from_format == to_format) {
    UE_LOG(LogBand, Display, TEXT("Formats must be different"));
    return false;
  }

  switch (from_format) {
    case FrameBuffer::Format::kGRAY:
      UE_LOG(LogBand, Display,
             TEXT("Grayscale format does not convert to other formats"));
      return false;
    case FrameBuffer::Format::kRGB:
      if (to_format == FrameBuffer::Format::kRGBA) {
        UE_LOG(LogBand, Display, TEXT("RGB format does not convert to RGBA"));
        return false;
      }
      return true;
    case FrameBuffer::Format::kRGBA:
    case FrameBuffer::Format::kNV12:
    case FrameBuffer::Format::kNV21:
    case FrameBuffer::Format::kYV12:
    case FrameBuffer::Format::kYV21:
      return true;
    default:
      UE_LOG(LogBand, Display, TEXT("Unsupported buffer format: %i"),
             from_format);
      return false;
  }
}

// Creation Methods
// -----------------------------------------------------------------

// Creates a FrameBuffer from raw RGBA buffer and passing arguments.
std::unique_ptr<FrameBuffer> CreateFromRgbaRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation, const uint64 timestamp,
    FrameBuffer::Stride stride) {
  if (stride == kDefaultStride) {
    stride.row_stride_bytes = dimension.width * kRgbaChannels;
    stride.pixel_stride_bytes = kRgbaChannels;
  }
  FrameBuffer::Plane input_plane = {/*buffer=*/input,
                                               /*stride=*/stride};
  return FrameBuffer::Create({input_plane}, dimension,
                             FrameBuffer::Format::kRGBA, orientation,
                             timestamp);
}

// Creates a FrameBuffer from raw RGB buffer and passing arguments.
std::unique_ptr<FrameBuffer> CreateFromRgbRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation, const uint64 timestamp,
    FrameBuffer::Stride stride) {
  if (stride == kDefaultStride) {
    stride.row_stride_bytes = dimension.width * kRgbChannels;
    stride.pixel_stride_bytes = kRgbChannels;
  }
  FrameBuffer::Plane input_plane = {/*buffer=*/input,
                                               /*stride=*/stride};
  return FrameBuffer::Create({input_plane}, dimension,
                             FrameBuffer::Format::kRGB, orientation, timestamp);
}

// Creates a FrameBuffer from raw grayscale buffer and passing arguments.
std::unique_ptr<FrameBuffer> CreateFromGrayRawBuffer(
    const uint8* input, FrameBuffer::Dimension dimension,
    FrameBuffer::Orientation orientation, const uint64 timestamp,
    FrameBuffer::Stride stride) {
  if (stride == kDefaultStride) {
    stride.row_stride_bytes = dimension.width * kGrayChannel;
    stride.pixel_stride_bytes = kGrayChannel;
  }
  FrameBuffer::Plane input_plane = {/*buffer=*/input,
                                               /*stride=*/stride};
  return FrameBuffer::Create({input_plane}, dimension,
                             FrameBuffer::Format::kGRAY, orientation,
                             timestamp);
}

// Creates a FrameBuffer from raw YUV buffer and passing arguments.
std::unique_ptr<FrameBuffer> CreateFromYuvRawBuffer(
    const uint8* y_plane, const uint8* u_plane, const uint8* v_plane,
    FrameBuffer::Format format, FrameBuffer::Dimension dimension,
    int row_stride_y, int row_stride_uv, int pixel_stride_uv,
    FrameBuffer::Orientation orientation, const uint64 timestamp) {
  const int pixel_stride_y = 1;
  std::vector<FrameBuffer::Plane> planes;
  if (format == FrameBuffer::Format::kNV21 ||
      format == FrameBuffer::Format::kYV12) {
    planes = {{y_plane, /*stride=*/{row_stride_y, pixel_stride_y}},
              {v_plane, /*stride=*/{row_stride_uv, pixel_stride_uv}},
              {u_plane, /*stride=*/{row_stride_uv, pixel_stride_uv}}};
  } else if (format == FrameBuffer::Format::kNV12 ||
             format == FrameBuffer::Format::kYV21) {
    planes = {{y_plane, /*stride=*/{row_stride_y, pixel_stride_y}},
              {u_plane, /*stride=*/{row_stride_uv, pixel_stride_uv}},
              {v_plane, /*stride=*/{row_stride_uv, pixel_stride_uv}}};
  } else {
    UE_LOG(LogBand, Display, TEXT("Input format is not YUV-like: %i"), format);
    return nullptr;
  }
  return FrameBuffer::Create(planes, dimension, format, orientation, timestamp);
}

std::unique_ptr<FrameBuffer> CreateFromRawBuffer(
    const uint8* buffer, FrameBuffer::Dimension dimension,
    const FrameBuffer::Format target_format,
    FrameBuffer::Orientation orientation, uint64 timestamp) {
  switch (target_format) {
    case FrameBuffer::Format::kNV12:
      return CreateFromOnePlaneNVRawBuffer(buffer, dimension, target_format,
                                           orientation, timestamp);
    case FrameBuffer::Format::kNV21:
      return CreateFromOnePlaneNVRawBuffer(buffer, dimension, target_format,
                                           orientation, timestamp);
    case FrameBuffer::Format::kYV12: {
      const FrameBuffer::Dimension uv_dimension =
          GetUvPlaneDimension(dimension, target_format);
      return CreateFromYuvRawBuffer(
          /*y_plane=*/buffer,
                      /*u_plane=*/
                      buffer + dimension.Size() + uv_dimension.Size(),
                      /*v_plane=*/buffer + dimension.Size(), target_format,
                      dimension,
                      /*row_stride_y=*/dimension.width, uv_dimension.width,
                      /*pixel_stride_uv=*/1, orientation, timestamp);
    }
    case FrameBuffer::Format::kYV21: {
      const FrameBuffer::Dimension uv_dimension =
          GetUvPlaneDimension(dimension, target_format);
      return CreateFromYuvRawBuffer(
          /*y_plane=*/buffer, /*u_plane=*/buffer + dimension.Size(),
                      /*v_plane=*/
                      buffer + dimension.Size() + uv_dimension.Size(),
                      target_format, dimension, /*row_stride_y=*/
                      dimension.width,
                      uv_dimension.width,
                      /*pixel_stride_uv=*/1, orientation, timestamp);
    }
    case FrameBuffer::Format::kRGBA:
      return CreateFromRgbaRawBuffer(buffer, dimension, orientation, timestamp);
    case FrameBuffer::Format::kRGB:
      return CreateFromRgbRawBuffer(buffer, dimension, orientation, timestamp);
    case FrameBuffer::Format::kGRAY:
      return CreateFromGrayRawBuffer(buffer, dimension, orientation, timestamp);
    default:
      UE_LOG(LogBand, Display, TEXT("Unsupported buffer format: %i"),
             target_format);
      return nullptr;
  }
}

std::unique_ptr<FrameBuffer> CreateFromAndroidCameraFrame(
    const UAndroidCameraFrame& camera_frame) {
  if (camera_frame.HasYUV()) {
    const UAndroidCameraFrame::NV12Frame& frame_data = camera_frame.GetData();
    return band::CreateFromYuvRawBuffer(
        frame_data.Y, frame_data.U, frame_data.V,
        band::FrameBuffer::Format::kNV12,
        {camera_frame.GetWidth(), camera_frame.GetHeight()},
        frame_data.YRowStride,
        frame_data.UVRowStride, frame_data.UVPixelStride);
  } else if (camera_frame.GetARGBBuffer()) {
    const uint8* frame_data = camera_frame.GetARGBBuffer();
    return band::CreateFromRgbaRawBuffer(
        frame_data, {camera_frame.GetWidth(), camera_frame.GetHeight()});
  } else {
    UE_LOG(LogBand, Display,
           TEXT("Camera frame is missing both YUV and ARGB buffer"));
    return nullptr;
  }
}
} // namespace Band
