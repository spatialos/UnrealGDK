// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerFlagsTestSpyObject.h"

void UWorkerFlagsTestSpyObject::SetFlagUpdated(const FString& FlagName, const FString& FlagValue)
{
	TimesUpdated++;

	return;
}

int UWorkerFlagsTestSpyObject::GetTimesFlagUpdated() const
{
	return TimesUpdated;
}
