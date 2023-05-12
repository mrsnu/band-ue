#include "BandBlueprintLibrary.h"

#include <functional>
#include <list>

#include "Band.h"
#include "BandLibrary.h"
#include "BandModel.h"
#include "Rect.h"


FString UBandBlueprintLibrary::GetVersion() {
  return FBandModule::Get().GetVersion();
}

/* BBox offsets: Offset of Left, Bottom, Right, Top (in this order) */
template <typename T>
TArray<FBandBoundingBox> GetDetectedBoxesInternal(
    EBandDetector DetectorType, TArray<UBandTensor*> Tensors, const size_t DetectionTensorIndex,
    TArray<int32> BBoxOffsets, const size_t ConfidenceTensorIndex,
    const size_t ConfidenceOffset, const size_t ClassTensorIndex,
    const size_t ClassOffset, const size_t LandmarkTensorIndex,
    const size_t LandmarkOffset, const UBandLabel* Label,
    const float ScoreThreshold, int32 LenBoxVector,
    int32 LenConfidenceVector, int32 LenClassVector, int32 NumLandmarks) {
  TArray<FBandBoundingBox> Boxes;
  if (Tensors.Num() <= DetectionTensorIndex) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given tensor index "
             "offset %llu out of bound %d"),
           DetectionTensorIndex, Tensors.Num());
    return Boxes;
  }

  if (Tensors.Num() <= ConfidenceTensorIndex) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given confidence "
             "index offset %llu out of bound %d"),
           ConfidenceTensorIndex, Tensors.Num());
    return Boxes;
  }

  if (Tensors.Num() <= LandmarkTensorIndex) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given landmark "
             "index offset %llu out of bound %d"),
           LandmarkTensorIndex, Tensors.Num());
    return Boxes;
  }

  // Assumption: (1) x [NumBoxes] x [LenBoxVector]
  const TArray<T>& DetectionResults =
      Tensors[DetectionTensorIndex]->GetBuffer<T>();
  const size_t BatchOffset = Tensors[DetectionTensorIndex]->Dim(0) == 1 ? 1 : 0;
  const int32 NumBoxes = Tensors[DetectionTensorIndex]->Dim(BatchOffset);
  LenBoxVector = LenBoxVector < 0
                   ? Tensors[DetectionTensorIndex]->Dim(1 + BatchOffset)
                   : LenBoxVector;
  
  // Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]
  const TArray<T>& ConfidenceResults =
      Tensors[ConfidenceTensorIndex]->GetBuffer<T>();
  const int32 NumTrueDims =
      Tensors[ConfidenceTensorIndex]->NumDims() - BatchOffset;
  LenConfidenceVector =
      LenConfidenceVector < 0
        ? (NumTrueDims >= 2
             ? Tensors[ConfidenceTensorIndex]->Dim(1 + BatchOffset)
             : 1)
        : LenConfidenceVector;
  
  // Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]  
  const TArray<T>& ClassResults = Tensors[ClassTensorIndex]->GetBuffer<T>();
  LenClassVector =
     LenClassVector < 0
       ? (NumTrueDims >= 2
            ? Tensors[ClassTensorIndex]->Dim(1 + BatchOffset)
            : 1)
       : LenClassVector;
  
  const TArray<T>& LandmarkResults = Tensors[LandmarkTensorIndex]->GetBuffer<T>();

  for (int32 BBoxOffset : BBoxOffsets) {
    if (BBoxOffset >= LenBoxVector) {
      UE_LOG(LogBand, Error,
             TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given box offset "
               "%d out of bound %d"),
             BBoxOffset, LenBoxVector);
      return Boxes;
    }
  }

  if (ConfidenceOffset >= LenConfidenceVector) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given confidence "
             "offset %llu out of bound %d"),
           ConfidenceOffset, LenConfidenceVector);
    return Boxes;
  }

  if (Label && ClassOffset >= LenClassVector) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given class offset "
             "%llu out of bound %d"),
           ClassOffset, LenClassVector);
    return Boxes;
  }

  if (LenBoxVector * NumBoxes != DetectionResults.Num()) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetDetectedBoxes: output_shape * "
             "output_offset != length of results (%d * %d = %d)"),
           LenBoxVector, NumBoxes, DetectionResults.Num());
    return Boxes;
  }

  for (int BoxIndex = 0; BoxIndex < NumBoxes; BoxIndex++) {
    const int BoxOffset = BoxIndex * LenBoxVector;
    const float Confidence =
        ConfidenceResults[BoxIndex * LenConfidenceVector + ConfidenceOffset];
    if (Confidence > ScoreThreshold) {
      FBandBoundingBox TempBox = FBandBoundingBox(
          Confidence, FRect({DetectionResults[BoxOffset + BBoxOffsets[0]],
                             DetectionResults[BoxOffset + BBoxOffsets[2]],
                             DetectionResults[BoxOffset + BBoxOffsets[3]],
                             DetectionResults[BoxOffset + BBoxOffsets[1]]}));

      if (Label) {
        TempBox.Label = Label->GetClassName(static_cast<int32>(
          ClassResults[BoxIndex * LenClassVector + ClassOffset]));
      }

      if (NumLandmarks >= 0) {
        for (int i = 0; i < NumLandmarks; i++) {
          float GlobalX = LandmarkResults[BoxIndex * LenBoxVector + LandmarkOffset + i * 2 + 0];
          float GlobalY = LandmarkResults[BoxIndex * LenBoxVector + LandmarkOffset + i * 2 + 1];
          float LocalX = (GlobalX - TempBox.Position.Left) / TempBox.GetWidth();
          float LocalY = (GlobalY - TempBox.Position.Top) / TempBox.GetHeight();
          TempBox.Landmark.Add(FBandLandmark(
              LocalX, LocalY, 0,
              LandmarkResults[BoxIndex * LenBoxVector + LandmarkOffset + NumLandmarks * 2]));
        }
        if (DetectorType == EBandDetector::RetinaFace) {
          TempBox.LandmarkEdge = {
            {0, 1},
            {1, 2},
            {2, 4},
            {4, 0},
            {0, 3},
            {1, 3},
            {2, 3},
            {4, 3}
          };
        }
      }
      Boxes.Push(TempBox);
    }
  }
  return Boxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::GetDetectedBoxes(
    UPARAM(ref) TArray<UBandTensor*> Tensors, EBandDetector DetectorType,
    UBandLabel* Label) {
  if (Tensors.Num() == 0) {
    UE_LOG(LogBand, Error, TEXT("Empty tensors to GetDetectedBoxes"));
    return {};
  }
  
  TArray<FBandBoundingBox> Boxes;
  size_t DetectionTensorIndex = 0;
  size_t ConfidenceTensorIndex = 0;
  size_t ClassTensorIndex = 0;
  size_t LandmarkTensorIndex = 0;

  TArray<int32> BBoxOffsets = {0, 1, 2, 3};
  size_t ConfidenceOffset = 15;
  size_t ClassOffset = 0;
  size_t LandmarkOffset = 0;
  
  int32 LenBoxVector = -1;
  int32 LenConfidenceVector = -1;
  int32 LenClassVector = -1;
  int32 LenLandmarkVector = -1;
  float ScoreThreshold = 0.2f;

  if (DetectorType == EBandDetector::SSD) {
    ConfidenceTensorIndex = 2;
    ConfidenceOffset = 0;
    ScoreThreshold = 0.5f;
    ClassTensorIndex = 1;
  } else if (DetectorType == EBandDetector::SSDMNetV2) {
    ConfidenceTensorIndex = 2;
    ConfidenceOffset = 0;
    ScoreThreshold = 0.5f;
    ClassTensorIndex = 1;
    BBoxOffsets = {1, 2, 3, 0}; // ymin, xmin, ymax, xmax
  } else if (DetectorType == EBandDetector::PalmDetection) {
    UE_LOG(LogBand, Log, TEXT("Not implemented detector type - PalmDetection"));
    return Boxes;
  } else if (DetectorType == EBandDetector::Unknown) {
    UE_LOG(LogBand, Error, TEXT("Unknown detector type"));
    return Boxes;
  } else if (DetectorType == EBandDetector::RetinaFace) {
    DetectionTensorIndex = 0;
    ConfidenceTensorIndex = 0;
    LandmarkTensorIndex = 0;
    ClassTensorIndex = 0;
    ScoreThreshold = 0.2f;
    LandmarkOffset = 4;
    ConfidenceOffset = 15;
    LenBoxVector = LenConfidenceVector = LenClassVector = 16;
    LenLandmarkVector = 5;
    BBoxOffsets = {0, 3, 2, 1}; // xmin, ymin, xmax, ymax
  }

  const EBandTensorType TensorType = Tensors[DetectionTensorIndex]->Type();
  switch (TensorType) {
    case EBandTensorType::Float32:
      Boxes = GetDetectedBoxesInternal<float>(
          DetectorType,
          Tensors,
          DetectionTensorIndex,
          BBoxOffsets,
          ConfidenceTensorIndex,
          ConfidenceOffset,
          ClassTensorIndex,
          ClassOffset,
          LandmarkTensorIndex,
          LandmarkOffset,
          Label,
          ScoreThreshold,
          LenBoxVector,
          LenConfidenceVector,
          LenClassVector,
          LenLandmarkVector);
      break;
    default:
      const UEnum* EnumPtr =
          FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
      UE_LOG(LogBand, Error, TEXT("GetDetectedBoxes: Unsupported tensor type %s"),
             *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
  }

  return Boxes;
}

template <typename T>
TArray<FVector2D> Get2DLandmarksInternal(
    TArray<UBandTensor*> Tensors, int LandmarkTensorIndex, int TensorDim,
    int NumLandmarks, TArray<int32> Offsets
    // Offsets[0] = X, Offsets[1] = Y, (Offsets[2] = Z)
    ) {
  TArray<FVector2D> Landmarks;

  if (Offsets.Num() < 2)
  // Will access only 2 (because we want to make 2D-vectors)
  {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: Get2DLandmarks: Number of offsets(%d) "
             "!= Dim(%d)"),
           Offsets.Num(), TensorDim);
    return Landmarks;
  }

  const TArray<T>& LandmarkResults =
      Tensors[LandmarkTensorIndex]->GetBuffer<T>();

  for (int Idx = 0; Idx < NumLandmarks; Idx++) {
    FVector2D TempVector =
        FVector2D(LandmarkResults[Idx * TensorDim + Offsets[0]],
                  LandmarkResults[Idx * TensorDim + Offsets[1]]);
    Landmarks.Push(TempVector);
  }

  return Landmarks;
}

// TODO: replace apps that use this function with GetLandmarks
TArray<FVector2D> UBandBlueprintLibrary::Get2DLandmarks(
    TArray<UBandTensor*> Tensors, EBandLandmark ModelType) {
  TArray<FVector2D> Landmarks;
  int LandmarkTensorIndex = 0;
  int NumLandmarks = 0;
  int TensorDim = 0;
  TArray<int32> Offsets = {0, 1}; // XYZ

  if (ModelType == EBandLandmark::MoveNetSingleThunder) {
    LandmarkTensorIndex = 0;
    NumLandmarks = 17;
    TensorDim = 3;
    Offsets = {1, 0}; // Tensor = [Y, X, Z]
  } else if (ModelType == EBandLandmark::FaceMeshMediapipe) {
    LandmarkTensorIndex = 0;
    NumLandmarks = 468;
    TensorDim = 3;
    Offsets = {0, 1}; // Tensor = [X, Y, Z]
  } else if (ModelType == EBandLandmark::Unknown) {
    UE_LOG(LogBand, Error, TEXT("Unknown landmark model type"));
    return Landmarks;
  }

  const EBandTensorType TensorType = Tensors[LandmarkTensorIndex]->Type();
  switch (TensorType) {
    case EBandTensorType::Float32:
      Landmarks = Get2DLandmarksInternal<float>(
          Tensors, LandmarkTensorIndex, TensorDim, NumLandmarks, Offsets);
      break;
    default:
      const UEnum* EnumPtr =
          FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
      UE_LOG(LogBand, Error, TEXT("Get2DLandmarks: Unsupported tensor type %s"),
             *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
  }

  return Landmarks;
}

template <typename T>
FBandBoundingBox GetLandmarksInternal(
    TArray<UBandTensor*> Tensors, int LandmarkTensorIndex, int TensorDim,
    int NumLandmarks, int NumCoords, TArray<int32> Offsets,
    // Offsets[0] = X, Offsets[1] = Y, (Offsets[2] = Z)
    int ConfOffset) {
  FBandBoundingBox Landmarks = {0, {0, 0, 0, 0}};

  if (Offsets.Num() != TensorDim)
  // Will access only 2 (because we want to make 2D-vectors)
  {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: GetLandmarks: Number of offsets(%d) != "
             "Dim(%d)"),
           Offsets.Num(), TensorDim);
    return Landmarks;
  }
  if (NumCoords > 3) {
    UE_LOG(LogBand, Error,
           TEXT("UBandBlueprintLibrary: Can't get more than 3 dimensions (only "
             "X, Y, Z)"));
    return Landmarks;
  }

  const TArray<T>& LandmarkResults =
      Tensors[LandmarkTensorIndex]->GetBuffer<T>();
  for (int Idx = 0; Idx < NumLandmarks; Idx++) {
    float Coords[3] = {0.0, 0.0, 0.0};
    float Conf = 0.0;
    for (int CIdx = 0; CIdx < NumCoords; CIdx++) {
      if (Offsets[CIdx] >= 0 &&
          Idx * TensorDim + Offsets[CIdx] < LandmarkResults.Num()) {
        Coords[CIdx] = LandmarkResults[Idx * TensorDim + Offsets[CIdx]];
      }
    }
    if (ConfOffset >= 0 &&
        Idx * TensorDim + ConfOffset < LandmarkResults.Num()) {
      // Valid offset
      Conf = LandmarkResults[Idx * TensorDim + ConfOffset];
    }
    FBandLandmark TempLandmark =
        FBandLandmark(Coords[0], Coords[1], Coords[2], Conf);
    Landmarks.Landmark.Push(TempLandmark);
  }
  return Landmarks;
}

FBandBoundingBox UBandBlueprintLibrary::GetLandmarks(
    TArray<UBandTensor*> Tensors, EBandLandmark ModelType) {
  FBandBoundingBox Landmarks = {0, {0, 0, 0, 0}};
  int LandmarkTensorIndex = 0;  
  int NumLandmarks = 0;
  int NumCoords = 0;
  int TensorDim = 0;
  TArray<int32> Offsets = {-1, -1, -1}; // XYZ, invalid offsets are negative
  int ConfOffset = 0;                   // Confidence offset
  
  if (Tensors.Num() == 0) {
    return Landmarks;
  }

  // reference: https://github.com/tensorflow/hub/blob/master/examples/colab/movenet.ipynb
  if (ModelType == EBandLandmark::MoveNetSingleThunder ||
      ModelType == EBandLandmark::MoveNetSingleLightning) {
    LandmarkTensorIndex = 0;
    NumLandmarks = 17;
    NumCoords = 2;
    TensorDim = 3;
    Offsets = {1, 0, -1}; // Tensor = [Y, X, Z]
    ConfOffset = 2;
  } else if (ModelType == EBandLandmark::HandLandmarkMediapipe) {
    LandmarkTensorIndex = 3;
    NumLandmarks = 21;
    NumCoords = 3;
    TensorDim = 3; // ?
    Offsets = {0, 1, 2};
    ConfOffset = -1;
  } else if (ModelType == EBandLandmark::FaceMeshMediapipe) {
    LandmarkTensorIndex = 0;
    NumLandmarks = 468;
    NumCoords = 3;
    TensorDim = 3;
    Offsets = {0, 1, 2};
    ConfOffset = -1;
  } else if (ModelType == EBandLandmark::Unknown) {
    UE_LOG(LogBand, Error, TEXT("Unknown landmark model type"));
    return Landmarks;
  }

  const EBandTensorType TensorType = Tensors[LandmarkTensorIndex]->Type();
  switch (TensorType) {
    case EBandTensorType::Float32:
      Landmarks = GetLandmarksInternal<float>(Tensors, LandmarkTensorIndex,
                                              TensorDim, NumLandmarks,
                                              NumCoords, Offsets, ConfOffset);
      break;
    default:
      const UEnum* EnumPtr =
          FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
      UE_LOG(LogBand, Error, TEXT("GetLandmarks: Unsupported tensor type %s"),
             *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
  }

  // Post-fix
  if (ModelType == EBandLandmark::MoveNetSingleThunder ||
      ModelType == EBandLandmark::MoveNetSingleLightning) {
    Landmarks.LandmarkEdge = {
        {0, 1},
        {0, 2},
        {1, 3},
        {2, 4},
        {0, 5},
        {0, 6},
        {5, 7},
        {7, 9},
        {6, 8},
        {8, 10},
        {5, 6},
        {5, 11},
        {6, 12},
        {11, 12},
        {11, 13},
        {13, 15},
        {12, 14},
        {14, 16}
    };
  } else if (ModelType == EBandLandmark::HandLandmarkMediapipe) {
    ConfOffset = 0;
    int ConfTensorIndex = 2;
    int ConfTensorDim = 1;
    Landmarks.Confidence =
        Tensors[ConfTensorIndex]
        ->GetBuffer<float>()[0 * ConfTensorDim + ConfOffset];
  } else if (ModelType == EBandLandmark::FaceMeshMediapipe) {
    Landmarks.LandmarkEdge = {
                                 {0, 267}, {7, 163}, {10, 338}, {13, 312}, {14, 317},
                                 {17, 314}, {21, 54}, {33, 7}, {33, 246}, {37, 0},
                                 {39, 37}, {40, 39}, {46, 53}, {52, 65}, {53, 52},
                                 {54, 103}, {58, 132}, {61, 146}, {61, 185}, {63, 105},
                                 {65, 55}, {66, 107}, {67, 109}, {70, 63}, {78, 95},
                                 {78, 191}, {80, 81}, {81, 82}, {82, 13}, {84, 17},
                                 {87, 14}, {88, 178}, {91, 181}, {93, 234}, {95, 88},
                                 {103, 67}, {105, 66}, {109, 10}, {127, 162}, {132, 93},
                                 {136, 172}, {144, 145}, {145, 153}, {146, 91}, {148, 176},
                                 {149, 150}, {150, 136}, {152, 148}, {153, 154}, {154, 155},
                                 {155, 133}, {157, 173}, {158, 157}, {159, 158}, {160, 159},
                                 {161, 160}, {162, 21}, {163, 144}, {172, 58}, {173, 133},
                                 {176, 149}, {178, 87}, {181, 84}, {185, 40}, {191, 80},
                                 {234, 127}, {246, 161}, {249, 390}, {251, 389}, {263, 249},
                                 {263, 466}, {267, 269}, {269, 270}, {270, 409}, {276, 283},
                                 {282, 295}, {283, 282}, {284, 251}, {288, 397}, {293, 334},
                                 {295, 285}, {296, 336}, {297, 332}, {300, 293}, {310, 415},
                                 {311, 310}, {312, 311}, {314, 405}, {317, 402}, {318, 324},
                                 {321, 375}, {323, 361}, {324, 308}, {332, 284}, {334, 296},
                                 {338, 297}, {356, 454}, {361, 288}, {365, 379}, {373, 374},
                                 {374, 380}, {375, 291}, {377, 152}, {378, 400}, {379, 378},
                                 {380, 381}, {381, 382}, {382, 362}, {384, 398}, {385, 384},
                                 {386, 385}, {387, 386}, {388, 387}, {389, 356}, {390, 373},
                                 {397, 365}, {398, 362}, {400, 377}, {402, 318}, {405, 321},
                                 {409, 291}, {415, 308}, {454, 323}, {466, 388}
                             };
    for (auto& landmark : Landmarks.Landmark) {
      for (int i = 0; i < 2; i++) {
        landmark.Point[i] /= 196;
      }
    }
  }
  return Landmarks;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::NMS(
    TArray<FBandBoundingBox> Boxes, const float IoU_Threshold) {
  TArray<FBandBoundingBox> NMSBoxes;
  TArray<FBandBoundingBox> PrevBoxes = TArray<FBandBoundingBox>(Boxes);

  while (PrevBoxes.Num() > 0) {
    PrevBoxes.Sort(
        [](const FBandBoundingBox& Box1, const FBandBoundingBox& Box2) {
          return Box1.Confidence > Box2.Confidence;
        });
    TArray<FBandBoundingBox> CurrBoxes = TArray<FBandBoundingBox>(PrevBoxes);
    FBandBoundingBox Max = CurrBoxes[0];
    NMSBoxes.Push(Max);
    PrevBoxes.Empty();

    for (int Index = 1; Index < CurrBoxes.Num(); Index++) {
      FBandBoundingBox Detection = CurrBoxes[Index];
      if (BoxIou(Max.Position, Detection.Position) < IoU_Threshold) {
        PrevBoxes.Push(Detection);
      }
    }
  }
  return NMSBoxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::FilterBoxesByConfidence(
    TArray<FBandBoundingBox> Boxes, const int MaxNumBoxes) {
  // Sort by descending order
  Boxes.Sort();
  if (Boxes.Num() > MaxNumBoxes) {
    Boxes.SetNum(MaxNumBoxes);
  }
  return Boxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::FilterBoxesByLabel(
    TArray<FBandBoundingBox> Boxes, FString Label) {
  TArray<FBandBoundingBox> FilteredBoxes;
  for (int Index = 0; Index < Boxes.Num(); Index++) {
    if (Boxes[Index].Label == Label) {
      FilteredBoxes.Push(Boxes[Index]);
    }
  }
  return FilteredBoxes;
}

void UBandBlueprintLibrary::PrintBox(FBandBoundingBox BoundingBox) {
  UE_LOG(LogBand, Log, TEXT("FBandBoundingBox: %f - (%f, %f, %f, %f)"),
         BoundingBox.Confidence, BoundingBox.Position.Left,
         BoundingBox.Position.Right, BoundingBox.Position.Top,
         BoundingBox.Position.Bottom);
}

FString UBandBlueprintLibrary::BoxToString(FBandBoundingBox BoundingBox) {
  return BoundingBox.ToString();
}

void UBandBlueprintLibrary::ParseRectF(FBandBoundingBox BoundingBox,
                                       const int ImageHeight,
                                       const int ImageWidth, float& PosX,
                                       float& PosY, float& SizeX,
                                       float& SizeY) {
  float Left = BoundingBox.Position.Left;
  float Right = BoundingBox.Position.Right;
  float Top = BoundingBox.Position.Top;
  float Bottom = BoundingBox.Position.Bottom;

  SizeX = (Right - Left) * ImageWidth;
  SizeY = (Bottom - Top) * ImageHeight;
  PosX = Left * ImageWidth;
  PosY = Top * ImageHeight;
}
