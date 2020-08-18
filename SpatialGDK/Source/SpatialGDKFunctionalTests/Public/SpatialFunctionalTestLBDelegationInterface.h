// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTestLBDelegationInterface.generated.h"

/**
 * Interface that can be added to Spatial's Load Balancing Strategies to allow you to
 * delegate Actors to specific Server Workers at runtime.
 * To guarantee that this delegation system works even when Server Workers don't have
 * complete World area of interest, we add USpatialFunctionalTestDelegationComponent
 * to Actors.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USpatialFunctionalTestLBDelegationInterface : public UInterface
{
	GENERATED_BODY()
};

class ISpatialFunctionalTestLBDelegationInterface
{
	GENERATED_BODY()

public:
	// Adds or changes the current Actor Delegation to WorkerId
	bool AddActorDelegation(AActor* Actor, VirtualWorkerId WorkerId, bool bPersistOnTestFinished = false);

	// Removes an Actor Delegation, which means that it will fallback to the Load Balancing Strategy
	bool RemoveActorDelegation(AActor* Actor);

	// If there's an Actor Delegation it will return True, and WorkerId and bIsPersistent will be set accordingly
	bool HasActorDelegation(AActor* Actor, VirtualWorkerId& WorkerId, bool& bIsPersistent);

	void RemoveAllActorDelegations(UWorld* World, bool bRemovePersistent = false);
};
