#include "BandLabelFactory.h"
#include "BandLabel.h"

UBandLabelFactory::UBandLabelFactory(
    const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer) {
  // TODO: Update to support construction from user-given texts
  bText = false;
  bCreateNew = false;
  bEditAfterNew = false;
  bEditorImport = true;

  Formats.Add(TEXT("label; Label format"));
  SupportedClass = UBandLabel::StaticClass();
}

bool UBandLabelFactory::FactoryCanImport(const FString& Filename) {
  return FPaths::GetExtension(Filename).Equals(TEXT("label"));
}

UObject* UBandLabelFactory::FactoryCreateFile(UClass* InClass,
                                              UObject* InParent, FName InName,
                                              EObjectFlags Flags,
                                              const FString& Filename,
                                              const TCHAR* Parms,
                                              FFeedbackContext* Warn,
                                              bool& bOutOperationCanceled) {
  FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName,
                                               Parms);
  UObject* Label = UBandLabel::LoadLabel(InParent, InName, Flags, Filename,
                                         bOutOperationCanceled);
  if (!Label) {
    bOutOperationCanceled = true;
  }
  FEditorDelegates::OnAssetPostImport.Broadcast(this, Label);
  return Label;
}
