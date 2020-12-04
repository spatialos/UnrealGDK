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

void USpatialWorkerFlags::SetWorkerFlag(const FString FlagName, FString FlagValue)
{
	FString& StoredFlagValue = WorkerFlags.FindOrAdd(FlagName);
	StoredFlagValue = MoveTemp(FlagValue);
	if (FOnWorkerFlagUpdated* OnWorkerFlagUpdatedPtr = WorkerFlagCallbacks.Find(FlagName))
	{
		OnWorkerFlagUpdatedPtr->Broadcast(FlagName, StoredFlagValue);
	}
	OnAnyWorkerFlagUpdated.Broadcast(FlagName, StoredFlagValue);
}

void USpatialWorkerFlags::RemoveWorkerFlag(const FString FlagName)
{
	WorkerFlags.Remove(FlagName);
}

void USpatialWorkerFlags::RegisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Remove(InDelegate);
}

void USpatialWorkerFlags::RegisterAndInvokeAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	RegisterAnyFlagUpdatedCallback(InDelegate);
	for (const auto& FlagValuePair : WorkerFlags)
	{
		InDelegate.Execute(FlagValuePair.Key, FlagValuePair.Value);
	}
}

void USpatialWorkerFlags::RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	FOnWorkerFlagUpdated& OnWorkerFlagUpdated = WorkerFlagCallbacks.FindOrAdd(InFlagName);
	OnWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	if (FOnWorkerFlagUpdated* OnWorkerFlagUpdatedPtr = WorkerFlagCallbacks.Find(InFlagName))
	{
		OnWorkerFlagUpdatedPtr->Remove(InDelegate);
	}
}

void USpatialWorkerFlags::RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	RegisterFlagUpdatedCallback(InFlagName, InDelegate);
	if (const FString* ValuePtr = WorkerFlags.Find(InFlagName))
	{
		InDelegate.Execute(InFlagName, *ValuePtr);
	}
}
