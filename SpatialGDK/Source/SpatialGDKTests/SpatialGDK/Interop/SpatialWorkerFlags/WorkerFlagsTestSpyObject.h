// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/SpatialWorkerFlags.h"

#include "WorkerFlagsTestSpyObject.generated.h"


UCLASS()
class UWorkerFlagsTestSpyObject : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION()
	void SetFlagUpdated(const FString& flagName, const FString& flagValue);

	int getTimesFlagUpdated() const;

private:

	int  timesUpdated = 0;
}; 
