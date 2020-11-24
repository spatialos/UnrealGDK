// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialWorkerFlags.h"

bool USpatialWorkerFlags::GetWorkerFlag(const FString& InFlagName, FString& OutFlagValue) const
{
	if (const FSpatialFlaginfo* InfoPtr = WorkerFlags.Find(InFlagName))
	{
		OutFlagValue = InfoPtr->Value;
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
		FSpatialFlaginfo& FlagInfo = WorkerFlags.FindOrAdd(NewName);
		FlagInfo.Value = NewValue;
		FlagInfo.Set = true;
		FlagInfo.FlagReady->Ready();
		FlagInfo.OnWorkerFlagUpdated.Broadcast(NewValue);
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
	for (const auto& FlagPair : WorkerFlags)
	{
		const FString& flagName = FlagPair.Key;
		const FSpatialFlaginfo& FlagInfo = FlagPair.Value;
		InDelegate.Execute(flagName, FlagInfo.Value);
	}
}

void USpatialWorkerFlags::UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	if (FSpatialFlaginfo* FlagInfo = WorkerFlags.Find(InFlagName))
	{
		FlagInfo->OnWorkerFlagUpdated.Remove(InDelegate);
	}
}

void USpatialWorkerFlags::AwaitFlagUpdated(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	const FSpatialFlaginfo& FlagInfo = WorkerFlags.FindOrAdd(InFlagName);
	FlagInfo.FlagReady->Await(FOnReady::CreateLambda([&FlagInfo, &InDelegate, &InFlagName](const FString& ErrorMessage) {
		if (!ErrorMessage.IsEmpty())
		{
			// Early out if USpatialWorkerFlags is destroyed before a flag is set no need to log an error
			return;
		}
		InDelegate.Execute(FlagInfo.Value);
	}));
}

void USpatialWorkerFlags::RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	FSpatialFlaginfo& FlagInfo = WorkerFlags.FindOrAdd(InFlagName);
	FlagInfo.OnWorkerFlagUpdated.Add(InDelegate);
}

void USpatialWorkerFlags::RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate)
{
	RegisterFlagUpdatedCallback(InFlagName, InDelegate);
	const FSpatialFlaginfo& FlagInfo = WorkerFlags.FindChecked(InFlagName);
	if (FlagInfo.Set)
	{
		InDelegate.Execute(FlagInfo.Value);
	}
}

void USpatialWorkerFlags::UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate)
{
	OnAnyWorkerFlagUpdated.Remove(InDelegate);
}
