#include "BandBlueprintLibrary.h"
#include "Band.h"
#include "BandModel.h"
#include "BandLibraryWrapper.h"
#include "BandLibrary.h"
#include "Rect.h"
#include "BandBoundingBox.h"

FString UBandBlueprintLibrary::GetVersion()
{
	return FBandModule::Get().GetVersion();
}

EBandStatus UBandBlueprintLibrary::Wait(int32 JobHandle, UPARAM(ref) TArray<UBandTensor*> OutputTensors)
{
	return FBandModule::Get().Wait(JobHandle, OutputTensors);
}

template <typename T>
TArray<UBandBoundingBox*> GetDetectedBoxesInternal(UBandTensor* Tensor, TArray<T> Results, const float Score_Threshold, TArray<int32> BBoxOffsets)
{
	TArray<UBandBoundingBox*> Boxes;

	const size_t BatchOffset = Tensor->Dim(0) == 1 ? 1 : 0;
	
	int32 OUTPUT_SHAPE_0 = Tensor->Dim(BatchOffset);
	int32 OUTPUT_SHAPE_1 = Tensor->Dim(1 + BatchOffset);

	if (OUTPUT_SHAPE_0 * OUTPUT_SHAPE_1 != Results.Num()) {
		UE_LOG(LogBand, Log, TEXT("UBandBlueprintLibrary: GetDetectedBoxes: output_shape * output_offset != length of results (%d * %d = %d)"), 
			OUTPUT_SHAPE_0, OUTPUT_SHAPE_1, Results.Num());
		return Boxes;
	}
	for (int Index = 0; Index < OUTPUT_SHAPE_0; Index++) {
		int Offset = Index * OUTPUT_SHAPE_1;
		float Confidence = Results[Offset + BBoxOffsets[0]];
		if (Confidence > Score_Threshold) {
			UBandBoundingBox* TempBox = NewObject<UBandBoundingBox>();
			TempBox->InitBandBoundingBox(Confidence, Results[Offset + BBoxOffsets[1]],
				Results[Offset + BBoxOffsets[2]],
				Results[Offset + BBoxOffsets[3]],
				Results[Offset + BBoxOffsets[4]]);
			Boxes.Push(TempBox);
		}
	}
	UE_LOG(LogBand, Log, TEXT("UBandRetinaFace: GetDetectedBoxes: Boxes' length %d"), Boxes.Num());
	return Boxes;
}

TArray<UBandBoundingBox*> UBandBlueprintLibrary::GetDetectedBoxes(UPARAM(ref) UBandTensor* Tensor, const float Score_Threshold, TArray<int32> BBoxOffsets)
{
	TArray<UBandBoundingBox*> Boxes;
	const EBandTensorType TensorType = Tensor->Type();
	switch (TensorType)
	{
	case EBandTensorType::Float32:
		Boxes = GetDetectedBoxesInternal<float>(Tensor, Tensor->GetF32Buffer(), Score_Threshold, BBoxOffsets);
		break;
	case EBandTensorType::UInt8:
		Boxes = GetDetectedBoxesInternal<uint8>(Tensor, Tensor->GetRawBuffer(), Score_Threshold, BBoxOffsets);
		break;
	default:
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBandTensorType"), true);
		UE_LOG(LogBand, Error, TEXT("UBandRetinaFace: Unsupported tensor type %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(TensorType)));
	}
	
	UE_LOG(LogBand, Log, TEXT("UBandRetinaFace: GetDetectedBoxes: Boxes' length %d"), Boxes.Num());
	return Boxes;
}

float BoxIntersection(FRect A, FRect B) {
	if (A.Right <= B.Left || B.Right <= A.Left) return 0.0f;
	if (A.Top <= B.Bottom || B.Top <= A.Bottom) return 0.0f;
	return (FGenericPlatformMath::Min(A.Right, B.Right) - FGenericPlatformMath::Max(A.Left, B.Left))
		* (FGenericPlatformMath::Min(A.Top, B.Top) - FGenericPlatformMath::Max(A.Bottom, B.Bottom));
}

float BoxUnion(FRect A, FRect B) {
	return (A.Right - A.Left) * (A.Top - A.Bottom)
		+ (B.Right - B.Left) * (B.Top - B.Bottom)
		- BoxIntersection(A, B);
}

float BoxIou(FRect A, FRect B) {
	return BoxIntersection(A, B) / BoxUnion(A, B);
}


TArray<UBandBoundingBox *> UBandBlueprintLibrary::NMS(TArray<UBandBoundingBox *> Boxes, const float IoU_Threshold) 
{
	TArray<UBandBoundingBox*> NMSBoxes;
	TArray<UBandBoundingBox*> PrevBoxes = TArray<UBandBoundingBox*>(Boxes);

	while (PrevBoxes.Num() > 0) {
		PrevBoxes.Sort([](const UBandBoundingBox& Box1, const UBandBoundingBox& Box2) {
			return Box1.Confidence < Box2.Confidence;
			});
		TArray<UBandBoundingBox*> CurrBoxes = TArray<UBandBoundingBox*>(PrevBoxes);
		UBandBoundingBox* Max = CurrBoxes[0];
		NMSBoxes.Push(Max);
		PrevBoxes.Empty();

		for (int Index = 1; Index < CurrBoxes.Num(); Index++) {
			UBandBoundingBox* Detection = CurrBoxes[Index];
			if (BoxIou(Max->Position, Detection->Position) < IoU_Threshold) {
				PrevBoxes.Push(Detection);
			}
		}
	}
	UE_LOG(LogBand, Log, TEXT("UBandRetinaFace: NMS: NMSBoxes' length %d"), NMSBoxes.Num());
	return NMSBoxes;
}