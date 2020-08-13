// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../TestActors/ReplicatedTestActorBase.h"
#include "TestPrepareShutdownListener.generated.h"

/**
* A replicated Actor with a Cube Mesh, used as a base for Actors used in tests.
*/
UCLASS()
class ATestPrepareShutdownListener : public AActor
{
	GENERATED_BODY()

public:
	ATestPrepareShutdownListener();

	bool RegisterCallback();

	UFUNCTION(BlueprintNativeEvent)
	void OnPrepareShutdown();

	int NativePrepareShutdownEventCount;

	UPROPERTY(BlueprintReadWrite, Category="Test Values")
	int BlueprintPrepareShutdownEventCount;
};
