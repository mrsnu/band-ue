#include "BandLandmark.h"

#include <cmath>

FBandLandmark::FBandLandmark(float X, float Y, float Z, float Confidence)
  : Confidence(Confidence), Point(X, Y, Z) {
}

float FBandLandmark::GetDistance(const FBandLandmark& Rhs) const {
  return std::sqrt(std::pow((Point.X - Rhs.Point.X), 2) +
                   std::pow((Point.Y - Rhs.Point.Y), 2) +
                   std::pow((Point.Z - Rhs.Point.Z), 2));
}

bool FBandLandmark::operator==(const FBandLandmark& rhs) const {
  return (Confidence == rhs.Confidence) && (Point == rhs.Point);
}

std::vector<size_t>
FBandLandmark::GetImportantLandmarks(EBandLandmark Type) {
  switch (Type) {
    // https://www.tensorflow.org/lite/examples/pose_estimation/overview
    case EBandLandmark::MoveNetSingleThunder:
    case EBandLandmark::MoveNetSingleLightning:
      return {
          0,     // nose
          5, 6,  // shoulder
          9, 10, // wrist
          13, 14
      }; // knee
    case EBandLandmark::HandLandmarkMediapipe:
      return {
          0, // wrist
          4, 8, 12, 16, 20
      }; // finger tips

    case EBandLandmark::FaceMeshMediapipe:
      // https://github.com/rcsmit/python_scripts_rcsmit/blob/master/extras/Gal_Gadot_by_Gage_Skidmore_4_5000x5921_annotated_black_letters.jpg
      return {
          0, 17,   // lips
          4,       // nose
          223, 443 // eyes
      };
    case EBandLandmark::Unknown:
    default:
      return {};;
  }
}
