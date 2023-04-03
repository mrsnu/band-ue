#include "BandLabel.h"
#include "Band.h"
#include "BandLibrary.h"

#include <fstream>
#include <string> // getline

FString UBandLabel::GetClassName(int32 Index) const {
  if (0 > Index || Labels.Num() < Index) {
    UE_LOG(LogBand, Error,
           TEXT("Trying to access label from index out of bound %d"), Index);
    return TEXT("");
  } else {
    return Labels[Index];
  }
}

UBandLabel* UBandLabel::LoadLabel(UObject* InParent, FName InName,
                                  EObjectFlags Flags, const FString& Filename,
                                  bool& bOutOperationCanceled) {
  std::ifstream InputFile(TCHAR_TO_ANSI(*Filename));
  if (InputFile.is_open()) {
    UBandLabel* Label = NewObject<UBandLabel>(InParent, InName, Flags);
    std::string ClassName;
    while (std::getline(InputFile, ClassName)) {
      Label->Labels.Push(ANSI_TO_TCHAR(ClassName.c_str()));
    }
    return Label;
  } else {
    bOutOperationCanceled = true;
    return nullptr;
  }
}
