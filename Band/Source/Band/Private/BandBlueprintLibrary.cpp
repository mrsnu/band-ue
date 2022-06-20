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
	const float ScoreThreshold)
{
	TArray<FBandBoundingBox> Boxes;
	// Assumption: (1) x [NumBoxes] x [LenBoxVector]
	const TArray<T>& DetectionResults = Tensors[DetectionTensorIndex]->GetBuffer<T>();
	const size_t BatchOffset = Tensors[DetectionTensorIndex]->Dim(0) == 1 ? 1 : 0;
	const int32 NumBoxes = Tensors[DetectionTensorIndex]->Dim(BatchOffset);
	const int32 LenBoxVector = Tensors[DetectionTensorIndex]->Dim(1 + BatchOffset);

	const int32 NumTrueDims = Tensors[ConfidenceTensorIndex]->NumDims() - BatchOffset;
	// Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]
	const TArray<T>& ConfidenceResults = Tensors[ConfidenceTensorIndex]->GetBuffer<T>();
	const int32 LenConfidenceVector = NumTrueDims >= 2 ?
		Tensors[ConfidenceTensorIndex]->Dim(1 + BatchOffset) : 1;
	// Assumption: (1) x [NumBoxes] x [LenConfidenceVector] or (1) x [NumBoxes]
	const TArray<T>& ClassResults = Tensors[ClassTensorIndex]->GetBuffer<T>();
	const int32 LenClassVector = NumTrueDims >= 2 ?
		Tensors[ClassTensorIndex]->Dim(1 + BatchOffset) : 1;

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
			FBandBoundingBox TempBox = FBandBoundingBox(Confidence,FRect({
				DetectionResults[BoxOffset + BBoxOffsets[0]],
				DetectionResults[BoxOffset + BBoxOffsets[2]],
				DetectionResults[BoxOffset + BBoxOffsets[3]],
				DetectionResults[BoxOffset + BBoxOffsets[1]]
			}));
			
			if (Label)
			{
				TempBox.Label = Label->GetClassName(static_cast<int32>(ClassResults[BoxIndex * LenClassVector + ClassOffset]));
			}
			
			Boxes.Push(TempBox);
		}
	}
	return Boxes;
}

/*
 * determine where the anchor points are scatterd.
 *   https://github.com/tensorflow/tfjs-models/blob/master/blazeface/src/face.ts
 */
std::list<FVector2D>
create_blazeface_anchors(int input_w, int input_h)
{
	std::list<FVector2D> anchor_list;
	/* ANCHORS_CONFIG  */
	int strides[2] = {input_w / 16, input_h / 8};
	int anchors[2] = {2,  6};

	int numtotal = 0;

	for (int i = 0; i < 2; i ++)
	{
		int stride = strides[i];
		int gridCols = (input_w + stride -1) / stride;
		int gridRows = (input_h + stride -1) / stride;
		int anchorNum = anchors[i];

		FVector2D anchor;
		for (int gridY = 0; gridY < gridRows; gridY ++)
		{
			anchor.Y = stride * (gridY + 0.5f);
			for (int gridX = 0; gridX < gridCols; gridX ++)
			{
				anchor.X = stride * (gridX + 0.5f);
				for (int n = 0; n < anchorNum; n ++)
				{
					anchor_list.push_back (anchor);
					numtotal ++;
				}
			}
		}
	}
	return anchor_list;
}


TArray<FBandBoundingBox> UBandBlueprintLibrary::GetDetectedBoxes(UPARAM(ref) TArray<UBandTensor*> Tensors, EBandDetector DetectorType, UBandLabel* Label)
{
	TArray<FBandBoundingBox> Boxes;
	
	if (DetectorType == EBandDetector::BlazeFace)
	{
		const int InputWidth = 256, InputHeight = 256;
		float ScoreThreshold = 0.75f;
		auto anchors = create_blazeface_anchors(InputWidth, InputHeight);
		
		FBandBoundingBox face_item;

		TArray<float> ScoresPtr, BoxesPtr;

		if (Tensors.Num() == 4)
		{
			ScoresPtr.Append(Tensors[0]->GetBuffer<float>());
			ScoresPtr.Append(Tensors[1]->GetBuffer<float>());
			BoxesPtr.Append(Tensors[2]->GetBuffer<float>());
			BoxesPtr.Append(Tensors[3]->GetBuffer<float>());
		}
		else if (Tensors.Num() == 2)
		{
			ScoresPtr.Append(Tensors[0]->GetBuffer<float>());
			BoxesPtr.Append(Tensors[1]->GetBuffer<float>());
		}

		int i = 0;
		for (auto itr = anchors.begin(); itr != anchors.end(); i ++, itr ++)
		{
			FVector2D anchor = *itr;
			float score0 = ScoresPtr[i];
			float score = 1.0f / (1.0f + exp(-score0));

			if (score > ScoreThreshold)
			{
				/* boundary box */
				float sx = BoxesPtr[i * 16];
				float sy = BoxesPtr[i * 16 + 1];
				float w  = BoxesPtr[i * 16 + 2];
				float h  = BoxesPtr[i * 16 + 3];

				float cx = sx + anchor.X;
				float cy = sy + anchor.Y;

				cx /= (float)InputWidth;
				cy /= (float)InputHeight;
				w  /= (float)InputWidth;
				h  /= (float)InputHeight;
				
				face_item.Confidence = score;
				face_item.Position.Top = cy - h * 0.5f;
				face_item.Position.Left = cx - w * 0.5f;
				face_item.Position.Bottom = cy + h * 0.5f;
				face_item.Position.Right = cx + w * 0.5f;

				const int kFaceKeyNum = 6;
				/* landmark positions (6 keys) */
				for (int j = 0; j < kFaceKeyNum; j ++)
				{
					float lx = BoxesPtr[i * 16 + 4 + (2 * j) + 0];
					float ly = BoxesPtr[i * 16 + 4 + (2 * j) + 1];
					lx += anchor.X;
					ly += anchor.Y;
					lx /= (float)InputWidth;
					ly /= (float)InputHeight;
					
					face_item.Landmark.Push({lx, ly, 0});
				}

				Boxes.Push(face_item);
			}
		}
		
		return Boxes;
	}
	else
	{
		// Default parameters from RetinaFace
		size_t DetectionTensorIndex = 0;
		TArray<int32> BBoxOffsets = {0, 1, 2, 3};
		size_t ConfidenceTensorIndex = 0;
		size_t ConfidenceOffset = 15;
		size_t ClassTensorIndex = 0;
		size_t ClassOffset = 0;
		float ScoreThreshold = 0.2f;
	
		if (DetectorType == EBandDetector::SSD)
		{
			ConfidenceTensorIndex = 2;
			ConfidenceOffset = 0;
			ScoreThreshold = 0.5f;
			ClassTensorIndex = 1;
		}
		else if (DetectorType == EBandDetector::Unknown)
		{
			UE_LOG(LogBand, Error, TEXT("Unknown detector type"));
			return Boxes;
		}
	
		const EBandTensorType TensorType = Tensors[DetectionTensorIndex]->Type();
		switch (TensorType)
		{
			case EBandTensorType::Float32:
				Boxes = GetDetectedBoxesInternal<float>(Tensors, DetectionTensorIndex, BBoxOffsets, ConfidenceTensorIndex, ConfidenceOffset, ClassTensorIndex, ClassOffset, Label, ScoreThreshold);
			break;
			default:
				const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
			UE_LOG(LogBand, Error, TEXT("Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
		}
	}
	
	return Boxes;
}

float BoxIntersection(FRect A, FRect B)
{
	if (A.Right <= B.Left || B.Right <= A.Left)
		return 0.0f;
	if (A.Top <= B.Bottom || B.Top <= A.Bottom)
		return 0.0f;
	return (FGenericPlatformMath::Min(A.Right, B.Right) - FGenericPlatformMath::Max(A.Left, B.Left))
		* (FGenericPlatformMath::Min(A.Top, B.Top) - FGenericPlatformMath::Max(A.Bottom, B.Bottom));
}

float BoxUnion(FRect A, FRect B)
{
	return (A.Right - A.Left) * (A.Top - A.Bottom)
		+ (B.Right - B.Left) * (B.Top - B.Bottom)
		- BoxIntersection(A, B);
}

float BoxIou(FRect A, FRect B)
{
	return BoxIntersection(A, B) / BoxUnion(A, B);
}

TArray<FBandBoundingBox> UBandBlueprintLibrary::NMS(TArray<FBandBoundingBox> Boxes, const float IoU_Threshold)
{
	TArray<FBandBoundingBox> NMSBoxes;
	TArray<FBandBoundingBox> PrevBoxes = TArray<FBandBoundingBox>(Boxes);

	while (PrevBoxes.Num() > 0)
	{
		PrevBoxes.Sort([](const FBandBoundingBox& Box1, const FBandBoundingBox& Box2) {
			return Box1.Confidence < Box2.Confidence;
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
