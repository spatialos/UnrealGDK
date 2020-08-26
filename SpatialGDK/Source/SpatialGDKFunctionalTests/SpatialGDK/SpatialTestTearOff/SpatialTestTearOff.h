// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestTearOff.generated.h"

class AReplicatedTearOffActor;
class AReplicatedTestActorBase;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestTearOff : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestTearOff();

	virtual void BeginPlay() override;

	// Variable used to reference the ReplicatedTearOffActor placed in the map.
	AReplicatedTearOffActor* StartupTearOffActor;

	// Variable used to reference the spawned AReplicatedTestActorBase.
	UPROPERTY()
	AReplicatedTestActorBase* SpawnedReplicatedActorBase;

	// The location where the ReplicatedActorBase is spawned.
	FVector ReplicatedTestActorBaseInitialLocation;

	// The location where the ReplicatedActorBase is moved before calling TearOff.
	FVector ReplicatedTestActorBaseMoveLocationBeforeTearOff;

	// THe location where the ReplicatedActorBase is moved after calling TearOff.
	FVector	ReplicatedTestActorBaseMoveLocationAfterTearOff;

	// Helper variable used for implementing the WorkerWaitForTimeStepDefinition.
	float TimerHelper;
};
