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
	void SetFlagUpdated(const FString& FlagName, const FString& FlagValue);

	UFUNCTION() // Defined in SpatialWorkerFlagsTest.cpp so the relevant code can be read together with the test
	void SetFlagUpdatedAndUnregisterCallback(const FString& FlagName, const FString& FlagValue);

	int GetTimesFlagUpdated() const;

	UPROPERTY()
	USpatialWorkerFlags* SpatialWorkerFlags;

	UPROPERTY()
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;

private:
	int TimesUpdated = 0;
};
