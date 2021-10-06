// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestPrepareShutdownListener.generated.h"

/**
 * Non-replicated actor that listens for the shutdown preparation trigger in C++ and reacts to it by incrementing the native event count.
 * Intended to be subclassed as a blueprint actor that listens to the event from blueprint logic as well, and increments the blueprint event
 * count.
 */
UCLASS()
class ATestPrepareShutdownListener : public AActor
{
	GENERATED_BODY()

public:
	ATestPrepareShutdownListener();

	UFUNCTION(BlueprintNativeEvent)
	bool RegisterCallback();

	UFUNCTION()
	void OnPrepareShutdownNative();

	int NativePrepareShutdownEventCount;

	UPROPERTY(BlueprintReadWrite, Category = "Test Values")
	int BlueprintPrepareShutdownEventCount;
};
