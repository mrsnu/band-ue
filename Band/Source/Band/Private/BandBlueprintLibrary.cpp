#include "BandBlueprintLibrary.h"

#include <functional>
#include <list>

#include "Band.h"
#include "BandModel.h"
#include "BandLibrary.h"
#include "Rect.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

/* BBox offsets: Offset of Left, Bottom, Right, Top (in this order) */
template <typename T>
TArray<FBandBoundingBox> GetDetectedBoxesInternal(
	TArray<UBandTensor*> Tensors,
	const size_t DetectionTensorIndex,
	TArray<int32> BBoxOffsets,
	const size_t ConfidenceTensorIndex,
	const size_t ConfidenceOffset,
	const size_t ClassTensorIndex,
	const size_t ClassOffset,
	const UBandLabel* Label,
	const float ScoreThreshold,
	int32 LenBoxVector = -1,
	int32 LenConfidenceVector = -1,
	int32 LenClassVector = -1)
{
	TArray<FBandBoundingBox> Boxes;
	if (Tensors.Num() <= DetectionTensorIndex)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given tensor index offset %d out of bound %d"),
			DetectionTensorIndex, Tensors.Num());
		return Boxes;
	}

	if (Tensors.Num() <= ConfidenceTensorIndex)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given confidence index offset %d out of bound %d"),
			ConfidenceTensorIndex, Tensors.Num());
		return Boxes;
	}

	// Assumption: (1) x [NumBoxes] x [LenBoxVector]
	const TArray<T>& DetectionResults = Tensors[DetectionTensorIndex]->GetBuffer<T>();
	const size_t BatchOffset = Tensors[DetectionTensorIndex]->Dim(0) == 1 ? 1 : 0;
	const int32 NumBoxes = Tensors[DetectionTensorIndex]->Dim(BatchOffset);
	LenBoxVector = LenBoxVector < 0 ? Tensors[DetectionTensorIndex]->Dim(1 + BatchOffset) : LenBoxVector;

	const int32 NumTrueDims = Tensors[ConfidenceTensorIndex]->NumDims() - BatchOffset;
	// Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]
	const TArray<T>& ConfidenceResults = Tensors[ConfidenceTensorIndex]->GetBuffer<T>();
	LenConfidenceVector = LenConfidenceVector < 0 ? (NumTrueDims >= 2 ? Tensors[ConfidenceTensorIndex]->Dim(1 + BatchOffset) : 1) : LenConfidenceVector;
	// Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]
	const TArray<T>& ClassResults = Tensors[ClassTensorIndex]->GetBuffer<T>();
	LenClassVector = LenClassVector < 0 ? (NumTrueDims >= 2 ? Tensors[ClassTensorIndex]->Dim(1 + BatchOffset) : 1) : LenClassVector;

	for (int32 BBoxOffset : BBoxOffsets)
	{
		if (BBoxOffset >= LenBoxVector)
		{
			UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given box offset %d out of bound %d"),
				BBoxOffset, LenBoxVector);
			return Boxes;
		}
	}

	if (ConfidenceOffset >= LenConfidenceVector)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given confidence offset %d out of bound %d"),
			ConfidenceOffset, LenConfidenceVector);
		return Boxes;
	}

	if (Label && ClassOffset >= LenClassVector)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: Given class offset %d out of bound %d"),
			ClassOffset, LenClassVector);
		return Boxes;
	}

	if (LenBoxVector * NumBoxes != DetectionResults.Num())
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: output_shape * output_offset != length of results (%d * %d = %d)"),
			LenBoxVector, NumBoxes, DetectionResults.Num());
		return Boxes;
	}

	for (int BoxIndex = 0; BoxIndex < NumBoxes; BoxIndex++)
	{
		const int BoxOffset = BoxIndex * LenBoxVector;
		const float Confidence = ConfidenceResults[BoxIndex * LenConfidenceVector + ConfidenceOffset];
		if (Confidence > ScoreThreshold)
		{
			FBandBoundingBox TempBox = FBandBoundingBox(Confidence, FRect({ DetectionResults[BoxOffset + BBoxOffsets[0]], DetectionResults[BoxOffset + BBoxOffsets[2]], DetectionResults[BoxOffset + BBoxOffsets[3]], DetectionResults[BoxOffset + BBoxOffsets[1]] }));

			if (Label)
			{
				TempBox.Label = Label->GetClassName(static_cast<int32>(ClassResults[BoxIndex * LenClassVector + ClassOffset]));
			}

			Boxes.Push(TempBox);
		}
	}
	return Boxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::GetDetectedBoxes(UPARAM(ref) TArray<UBandTensor*> Tensors, EBandDetector DetectorType, UBandLabel* Label)
{
	TArray<FBandBoundingBox> Boxes;
	// Default parameters from RetinaFace
	size_t DetectionTensorIndex = 0;
	TArray<int32> BBoxOffsets = { 0, 1, 2, 3 };
	size_t ConfidenceTensorIndex = 0;
	size_t ConfidenceOffset = 15;
	size_t ClassTensorIndex = 0;
	size_t ClassOffset = 0;
	int32 LenBoxVector = -1;
	int32 LenConfidenceVector = -1;
	int32 LenClassVector = -1;
	float ScoreThreshold = 0.2f;

	if (DetectorType == EBandDetector::SSD)
	{
		ConfidenceTensorIndex = 2;
		ConfidenceOffset = 0;
		ScoreThreshold = 0.5f;
		ClassTensorIndex = 1;
	}
	else if (DetectorType == EBandDetector::SSDMNetV2)
	{
		ConfidenceTensorIndex = 2;
		ConfidenceOffset = 0;
		ScoreThreshold = 0.5f;
		ClassTensorIndex = 1;
		BBoxOffsets = { 1, 0, 3, 2 }; // ymin, xmin, ymax, xmax
	}
	else if (DetectorType == EBandDetector::PalmDetection)
	{
		UE_LOG(LogBand, Log, TEXT("Not implemented detector type - PalmDetection"));
		return Boxes;
	}
	else if (DetectorType == EBandDetector::Unknown)
	{
		UE_LOG(LogBand, Error, TEXT("Unknown detector type"));
		return Boxes;
	}
	else if (DetectorType == EBandDetector::RetinaFace)
	{
		DetectionTensorIndex = 0;
		ConfidenceTensorIndex = 0;
		ClassTensorIndex = 0;
		ScoreThreshold = 0.7f;
		ConfidenceOffset = 15;
		LenBoxVector = LenConfidenceVector = LenClassVector = 16;
		BBoxOffsets = { 0, 3, 2, 1 }; // xmin, ymin, xmax, ymax
	}

	const EBandTensorType TensorType = Tensors[DetectionTensorIndex]->Type();
	switch (TensorType)
	{
		case EBandTensorType::Float32:
			Boxes = GetDetectedBoxesInternal<float>(Tensors, DetectionTensorIndex, BBoxOffsets, ConfidenceTensorIndex, ConfidenceOffset, ClassTensorIndex, ClassOffset, Label, ScoreThreshold, LenBoxVector, LenConfidenceVector, LenClassVector);
			break;
		default:
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			UE_LOG(LogBand, Error, TEXT("Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
	}

	return Boxes;
}

template <typename T>
TArray<FVector2D> Get2DLandmarksInternal(TArray<UBandTensor*> Tensors,
	int LandmarkTensorIndex,
	int TensorDim,
	int NumLandmarks,
	TArray<int32> Offsets // Offsets[0] = X, Offsets[1] = Y, (Offsets[2] = Z)
)
{
	TArray<FVector2D> Landmarks;

	if (Offsets.Num() < 2) // Will access only 2 (because we want to make 2D-vectors)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: Get2DLandmarks: Number of offsets(%d) != Dim(%d)"),
			Offsets.Num(), TensorDim);
		return Landmarks;
	}

	const TArray<T>& LandmarkResults = Tensors[LandmarkTensorIndex]->GetBuffer<T>();

	for (int Idx = 0; Idx < NumLandmarks; Idx++)
	{
		FVector2D TempVector = FVector2D(LandmarkResults[Idx * TensorDim + Offsets[0]], LandmarkResults[Idx * TensorDim + Offsets[1]]);
		Landmarks.Push(TempVector);
	}

	return Landmarks;
}

// TODO: replace apps that use this function with GetLandmarks
TArray<FVector2D> UBandBlueprintLibrary::Get2DLandmarks(TArray<UBandTensor*> Tensors, EBandLandmark ModelType)
{
	TArray<FVector2D> Landmarks;
	int LandmarkTensorIndex = 0;
	int NumLandmarks = 0;
	int TensorDim = 0;
	TArray<int32> Offsets = { 0, 1 }; // XYZ

	if (ModelType == EBandLandmark::MoveNetSingleThunder)
	{
		LandmarkTensorIndex = 0;
		NumLandmarks = 17;
		TensorDim = 3;
		Offsets = { 1, 0 }; // Tensor = [Y, X, Z]
	}
	else if (ModelType == EBandLandmark::Unknown)
	{
		UE_LOG(LogBand, Error, TEXT("Unknown landmark model type"));
		return Landmarks;
	}

	const EBandTensorType TensorType = Tensors[LandmarkTensorIndex]->Type();
	switch (TensorType)
	{
		case EBandTensorType::Float32:
			Landmarks = Get2DLandmarksInternal<float>(Tensors, LandmarkTensorIndex, TensorDim, NumLandmarks, Offsets);
			break;
		default:
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			UE_LOG(LogBand, Error, TEXT("Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
	}

	return Landmarks;
}

template <typename T>
FBandBoundingBox GetLandmarksInternal(TArray<UBandTensor*> Tensors,
	int LandmarkTensorIndex,
	int TensorDim,
	int NumLandmarks,
	int NumCoords,
	TArray<int32> Offsets, // Offsets[0] = X, Offsets[1] = Y, (Offsets[2] = Z)
	int ConfOffset
)
{
	FBandBoundingBox Landmarks = {0, {0, 0, 0, 0}};

	if (Offsets.Num() < 2) // Will access only 2 (because we want to make 2D-vectors)
		{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: GetLandmarks: Number of offsets(%d) != Dim(%d)"),
			Offsets.Num(), TensorDim);
		return Landmarks;
		}
	if(NumCoords > 3)
	{
		UE_LOG(LogBand, Error, TEXT("UBandBlueprintLibrary: Can't get more than 3 dimensions (only X, Y, Z)"));
		return Landmarks;
	}

	const TArray<T>& LandmarkResults = Tensors[LandmarkTensorIndex]->GetBuffer<T>();

	for (int Idx = 0; Idx < NumLandmarks; Idx++)
	{
		float Coords[3] = {0.0, 0.0, 0.0};
		float Conf = 0.0;
		for(int CIdx = 0; CIdx < NumCoords; CIdx++)
		{
			if(Offsets[CIdx] >= 0){
				Coords[CIdx] = LandmarkResults[Idx * TensorDim + Offsets[CIdx]];
			}
		}
		if(ConfOffset >= 0){ // Valid offset
			Conf = LandmarkResults[Idx * TensorDim + ConfOffset];
		}
		FBandLandmark TempLandmark = FBandLandmark(Coords[0], Coords[1], Coords[2], Conf);
		Landmarks.Landmark.Push(TempLandmark);
	}
	
	return Landmarks;
}

FBandBoundingBox UBandBlueprintLibrary::GetLandmarks(TArray<UBandTensor*> Tensors, EBandLandmark ModelType)
{
	FBandBoundingBox Landmarks = {0, {0, 0, 0, 0}};
	int LandmarkTensorIndex = 0;
	int NumLandmarks = 0;
	int NumCoords = 0;
	int TensorDim = 0;
	TArray<int32> Offsets = { -1, -1, -1 }; // XYZ, invalid offsets are negative
	int ConfOffset = 0; // Confidence offset

	if(Tensors.Num() == 0) return Landmarks;

	if (ModelType == EBandLandmark::MoveNetSingleThunder || ModelType == EBandLandmark::MoveNetSingleLightning)
	{
		LandmarkTensorIndex = 0;
		NumLandmarks = 17;
		NumCoords = 2;
		TensorDim = 3;
		Offsets = { 1, 0, -1 }; // Tensor = [Y, X, Z]
		ConfOffset = 2;
	}
	else if (ModelType == EBandLandmark::Unknown)
	{
		UE_LOG(LogBand, Error, TEXT("Unknown landmark model type"));
		return Landmarks;
	}

	const EBandTensorType TensorType = Tensors[LandmarkTensorIndex]->Type();
	switch (TensorType)
	{
	case EBandTensorType::Float32:
		Landmarks = GetLandmarksInternal<float>(Tensors, LandmarkTensorIndex, TensorDim, NumLandmarks, NumCoords, Offsets, ConfOffset);
		break;
	default:
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
		UE_LOG(LogBand, Error, TEXT("Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
	}

	return Landmarks;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::NMS(TArray<FBandBoundingBox> Boxes, const float IoU_Threshold)
{
	TArray<FBandBoundingBox> NMSBoxes;
	TArray<FBandBoundingBox> PrevBoxes = TArray<FBandBoundingBox>(Boxes);

	while (PrevBoxes.Num() > 0)
	{
		PrevBoxes.Sort([](const FBandBoundingBox& Box1, const FBandBoundingBox& Box2) {
			return Box1.Confidence > Box2.Confidence;
		});
		TArray<FBandBoundingBox> CurrBoxes = TArray<FBandBoundingBox>(PrevBoxes);
		FBandBoundingBox Max = CurrBoxes[0];
		NMSBoxes.Push(Max);
		PrevBoxes.Empty();

		for (int Index = 1; Index < CurrBoxes.Num(); Index++)
		{
			FBandBoundingBox Detection = CurrBoxes[Index];
			if (BoxIou(Max.Position, Detection.Position) < IoU_Threshold)
			{
				PrevBoxes.Push(Detection);
			}
		}
	}

	return NMSBoxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::FilterBoxesByConfidence(TArray<FBandBoundingBox> Boxes, const int MaxNumBoxes)
{
	// Sort by descending order
	Boxes.Sort();
	if (Boxes.Num() > MaxNumBoxes)
	{
		Boxes.SetNum(MaxNumBoxes);
	}
	return Boxes;
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::FilterBoxesByLabel(TArray<FBandBoundingBox> Boxes, FString Label)
{
	TArray<FBandBoundingBox> FilteredBoxes;
	for (int Index = 0; Index < Boxes.Num(); Index++)
	{
		if (Boxes[Index].Label == Label)
		{
			FilteredBoxes.Push(Boxes[Index]);
		}
	}
	return FilteredBoxes;
}

void UBandBlueprintLibrary::PrintBox(FBandBoundingBox BoundingBox)
{
	UE_LOG(LogBand, Log, TEXT("FBandBoundingBox: %f - (%f, %f, %f, %f)"), BoundingBox.Confidence,
		BoundingBox.Position.Left, BoundingBox.Position.Right, BoundingBox.Position.Top, BoundingBox.Position.Bottom);
}

void UBandBlueprintLibrary::ParseRectF(FBandBoundingBox BoundingBox, const int ImageHeight, const int ImageWidth, float& PosX, float& PosY, float& SizeX, float& SizeY)
{

	float Left = BoundingBox.Position.Left;
	float Right = BoundingBox.Position.Right;
	float Top = BoundingBox.Position.Top;
	float Bottom = BoundingBox.Position.Bottom;

	SizeX = (Right - Left) * ImageWidth;
	SizeY = (Bottom - Top) * ImageHeight;

	PosX = Left * ImageWidth;
	PosY = Top * ImageHeight;

	if (SizeX < 0)
	{ // Shift PosX to the left by SizeX
		PosX += SizeX;
	}
	if (SizeY < 0)
	{ // Shift PoxY upwards by SizeY
		PosY += SizeY;
	}

	SizeX = FGenericPlatformMath::Abs(SizeX);
	SizeY = FGenericPlatformMath::Abs(SizeY);
}
