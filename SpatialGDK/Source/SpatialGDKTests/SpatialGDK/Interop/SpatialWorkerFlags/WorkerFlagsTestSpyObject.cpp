// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerFlagsTestSpyObject.h"

void  UWorkerFlagsTestSpyObject::SetFlagUpdated(const FString& flagName, const FString& flagValue)
{
	timesUpdated++;

	return;
}

int UWorkerFlagsTestSpyObject::getTimesFlagUpdated() const
{
	return timesUpdated;
}
