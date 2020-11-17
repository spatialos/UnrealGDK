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
	void SetAnyFlagUpdated(const FString& FlagName, const FString& FlagValue);
	UFUNCTION()
	void SetFlagUpdated(const FString& FlagValue);

	int GetTimesFlagUpdated() const;

private:
	int TimesUpdated = 0;
};
