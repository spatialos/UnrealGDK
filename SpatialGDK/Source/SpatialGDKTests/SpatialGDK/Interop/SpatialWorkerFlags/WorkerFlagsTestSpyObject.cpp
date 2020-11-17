// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerFlagsTestSpyObject.h"

void UWorkerFlagsTestSpyObject::SetAnyFlagUpdated(const FString& FlagName, const FString& FlagValue)
{
	TimesUpdated++;

	return;
}

void UWorkerFlagsTestSpyObject::SetFlagUpdated(const FString& FlagValue)
{
	TimesUpdated++;

	return;
}

int UWorkerFlagsTestSpyObject::GetTimesFlagUpdated() const
{
	return TimesUpdated;
}
