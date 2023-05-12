#include "BandLandmark.h"

#include <cmath>

float FBandLandmark::GetDistance(const FBandLandmark& Rhs) const {
  return std::sqrt(std::pow((Point.X - Rhs.Point.X), 2) +
                   std::pow((Point.Y - Rhs.Point.Y), 2) +
                   std::pow((Point.Z - Rhs.Point.Z), 2));
}

