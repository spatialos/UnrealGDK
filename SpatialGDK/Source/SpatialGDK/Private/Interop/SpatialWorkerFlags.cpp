// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialWorkerFlags.h"

bool USpatialWorkerFlags::GetWorkerFlag(const FString& InFlagName, FString& OutFlagValue) const
{
	if (const FString* ValuePtr = WorkerFlags.Find(InFlagName))
	{
		OutFlagValue = *ValuePtr;
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
		FString& FlagValue = WorkerFlags.FindOrAdd(NewName);
		FlagValue = NewValue;
		if (FOnWorkerFlagUpdated* CallbackPtr = WorkerFlagsCallbacks.Find(NewName))
		{
			CallbackPtr->Broadcast(NewValue);
		}
		OnAnyWorkerFlagUpdated.Broadcast(NewName, NewValue);
	}
	else
	{
		WorkerFlags.Remove(NewName);
	}
}

void USpatialWorkerFlags::RegisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::RegisterAndInvokeAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	RegisterAnyFlagUpdatedCallback(InDelegate);
	for (const auto& FlagValuePair : WorkerFlags)
	{
		InDelegate.Execute(FlagValuePair.Key, FlagValuePair.Value);
	}
}

void USpatialWorkerFlags::UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	if (FOnWorkerFlagUpdated* CallbackPtr = WorkerFlagsCallbacks.Find(InFlagName))
	{
		CallbackPtr->Remove(InDelegate);
	}
}

void USpatialWorkerFlags::RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	FOnWorkerFlagUpdated& Callback = WorkerFlagsCallbacks.FindOrAdd(InFlagName);
	Callback.Add(InDelegate);
}

void USpatialWorkerFlags::RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	RegisterFlagUpdatedCallback(InFlagName, InDelegate);
	if (const FString* ValuePtr = WorkerFlags.Find(InFlagName))
	{
		InDelegate.Execute(*ValuePtr);
	}
}

void USpatialWorkerFlags::UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Remove(InDelegate);
}
