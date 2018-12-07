// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialWorkerFlags.h"
#include <WorkerSDK/improbable/c_worker.h>

TMap<FString, FString> USpatialWorkerFlags::WorkerFlags;

bool USpatialWorkerFlags::GetWorkerFlag(const FString& Name, FString& OutValue)
{
	if (FString* ValuePtr = WorkerFlags.Find(Name))
	{
		OutValue = *ValuePtr;
		return true;
	}

	return false;
}

void USpatialWorkerFlags::ApplyWorkerFlagUpdate(const Worker_FlagUpdateOp& Op)
{
	FString NewName = FString(UTF8_TO_TCHAR(Op.name));

	if (Op.value != nullptr)
	{
		FString NewValue = FString(UTF8_TO_TCHAR(Op.value));
		FString& ValueFlag = WorkerFlags.FindOrAdd(NewName);
		ValueFlag = NewValue;
	}
	else
	{
		WorkerFlags.Remove(NewName);
	}
}
