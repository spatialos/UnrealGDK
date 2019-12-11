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
	void SetFlagUpdated(FString flagName, FString flagValue);

	bool isFlagUpdated = false;
}; 
