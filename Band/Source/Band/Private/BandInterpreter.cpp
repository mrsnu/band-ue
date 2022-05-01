// Fill out your copyright notice in the Description page of Project Settings.


#include "BandInterpreter.h"

void UBandInterpreter::Initialize(Band::TfLiteInterpreter* Handle)
{
	InterpreterHandle = Handle;
}

Band::TfLiteInterpreter* UBandInterpreter::GetHandle() const
{
	return InterpreterHandle;
}

UBandInterpreter::operator Band::TfLiteInterpreter*() const
{
	return InterpreterHandle;
}
