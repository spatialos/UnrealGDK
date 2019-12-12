// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/SpatialWorkerFlags.h"

#include "DummyObject.generated.h"


UCLASS()
class UDummyObject : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION()
	void SetFlagUpdated(const FString& flagName, const FString& flagValue);

	bool isFlagUpdated = false;
	int  timesUpdated = 0;
}; 
