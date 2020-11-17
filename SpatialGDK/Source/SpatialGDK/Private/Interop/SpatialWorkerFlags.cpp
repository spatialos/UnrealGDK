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
		FString& ValueFlag = WorkerFlags.FindOrAdd(NewName);
		ValueFlag = NewValue;
		OnAnyWorkerFlagUpdated.Broadcast(NewName, NewValue);
		if (FOnWorkerFlagUpdated* OnWorkerFlagUpdated = OnWorkerFlagUpdatedMap.Find(NewName))
		{
			OnWorkerFlagUpdated->Broadcast(NewValue);
		}
	}
	else
	{
		WorkerFlags.Remove(NewName);
	}
}

void USpatialWorkerFlags::RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	RegisterFlagUpdatedCallback(InFlagName, InDelegate);
	if (FString* ValueFlag = WorkerFlags.Find(InFlagName))
	{
		InDelegate.Execute(*ValueFlag);
	}
}

void USpatialWorkerFlags::RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	FOnWorkerFlagUpdated& OnWorkerFlagUpdated = OnWorkerFlagUpdatedMap.FindOrAdd(InFlagName);
	OnWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	if (FOnWorkerFlagUpdated* OnWorkerFlagUpdated = OnWorkerFlagUpdatedMap.Find(InFlagName))
	{
		OnWorkerFlagUpdated->Remove(InDelegate);
	}
}

void USpatialWorkerFlags::RegisterAndInvokeAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	RegisterAnyFlagUpdatedCallback(InDelegate);
	for (const auto& flagPair : WorkerFlags)
	{
		OnAnyWorkerFlagUpdated.Broadcast(flagPair.Key, flagPair.Value);
	}
}

void USpatialWorkerFlags::RegisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Remove(InDelegate);
}
