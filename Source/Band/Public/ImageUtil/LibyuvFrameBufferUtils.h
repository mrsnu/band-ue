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

#include "ImageUtil/FrameBuffer.h"
#include "ImageUtil/FrameBufferUtilsInterface.h"

namespace band {
// Libyuv image processing engine conforms to FrameBufferUtilsInterface.
// Although this class provides public APIs, it is recommended to use the public
// APIs defined in frame_buffer_utils.h for higher level abstraction and better
// functionality support.
class LibyuvFrameBufferUtils : public FrameBufferUtilsInterface {
public:
  LibyuvFrameBufferUtils() = default;
  virtual ~LibyuvFrameBufferUtils() override = default;

  // Crops input `buffer` to the specified subregions and resizes the cropped
  // region to the target image resolution defined by the `output_buffer`.
  //
  // (x0, y0) represents the top-left point of the buffer.
  // (x1, y1) represents the bottom-right point of the buffer.
  //
  // Crop region dimensions must be equal or smaller than input `buffer`
  // dimensions.
  virtual bool Crop(const FrameBuffer& buffer, int x0, int y0, int x1, int y1,
                    FrameBuffer* output_buffer) override;

  // Resizes `buffer` to the size of the given `output_buffer`.
  virtual bool Resize(const FrameBuffer& buffer,
                      FrameBuffer* output_buffer) override;

  // Rotates `buffer` counter-clockwise by the given `angle_deg` (in degrees).
  //
  // The given angle must be a multiple of 90 degrees.
  virtual bool Rotate(const FrameBuffer& buffer, int angle_deg,
                      FrameBuffer* output_buffer) override;

  // Flips `buffer` horizontally.
  virtual bool FlipHorizontally(const FrameBuffer& buffer,
                                FrameBuffer* output_buffer) override;

  // Flips `buffer` vertically.
  virtual bool FlipVertically(const FrameBuffer& buffer,
                              FrameBuffer* output_buffer) override;

  // Converts `buffer`'s format to the format of the given `output_buffer`.
  //
  // Grayscale format cannot be converted to other formats.
  virtual bool Convert(const FrameBuffer& buffer,
                       FrameBuffer* output_buffer) override;
};
} // namespace Band
