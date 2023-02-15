#include "BandModelFactory.h"
#include "BandModel.h"

UBandModelFactory::UBandModelFactory(
    const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer) {
  bText = false;
  bCreateNew = false;
  bEditAfterNew = false;
  bEditorImport = true;

  Formats.Add(TEXT("tflite; TfLite model format"));
  SupportedClass = UBandModel::StaticClass();
}

bool UBandModelFactory::FactoryCanImport(const FString& Filename) {
  return FPaths::GetExtension(Filename).Equals(TEXT("tflite"));
}

UObject* UBandModelFactory::FactoryCreateBinary(
    UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
    UObject* Context, const TCHAR* Type, const uint8*& Buffer,
    const uint8* BufferEnd, FFeedbackContext* Warn) {
  FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName,
                                               Type);
  UObject* Model = UBandModel::LoadModel(InParent, InName, Flags, Buffer,
                                         (BufferEnd - Buffer) + 1);
  FEditorDelegates::OnAssetPostImport.Broadcast(this, Model);
  return Model;
}
