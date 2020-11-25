// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerFlagsTestSpyObject.h"

void UWorkerFlagsTestSpyObject::SetAnyFlagUpdated(const FString& FlagName, const FString& FlagValue)
{
	SetFlagUpdated(FlagValue, FlagValue);
}

void UWorkerFlagsTestSpyObject::SetFlagUpdated(const FString& FlagName, const FString& FlagValue)
{
	++TimesUpdated;
}

int UWorkerFlagsTestSpyObject::GetTimesFlagUpdated() const
{
	return TimesUpdated;
}
