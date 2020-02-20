// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialWorkerFlags.h"

TMap<FString, FString> USpatialWorkerFlags::WorkerFlags;
FOnWorkerFlagsUpdated USpatialWorkerFlags::OnWorkerFlagsUpdated;

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
		OnWorkerFlagsUpdated.Broadcast(NewName, NewValue);
	}
	else
	{
		WorkerFlags.Remove(NewName);
	}
}
FOnWorkerFlagsUpdated& USpatialWorkerFlags::GetOnWorkerFlagsUpdated()
{
	return OnWorkerFlagsUpdated;
}

void USpatialWorkerFlags::BindToOnWorkerFlagsUpdated(const FOnWorkerFlagsUpdatedBP& InDelegate)
{
	OnWorkerFlagsUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::UnbindFromOnWorkerFlagsUpdated(const FOnWorkerFlagsUpdatedBP& InDelegate)
{
	OnWorkerFlagsUpdated.Remove(InDelegate);
}
