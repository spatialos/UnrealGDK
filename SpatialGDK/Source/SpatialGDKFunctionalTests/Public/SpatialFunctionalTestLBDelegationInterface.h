// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTestLBDelegationInterface.generated.h"

/**
 * A 2 by 2 (rows by columns) load balancing strategy for testing zoning features.
 * Has a 500 unit interest border, so the shared interest between workers should be small.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USpatialFunctionalTestLBDelegationInterface : public UInterface
{
	GENERATED_BODY()
};

struct FSpatialFunctionalTestActorDelegation
{
	uint32 Id = 0;
	TWeakObjectPtr<AActor> ActorPtr = nullptr;
	bool bPersistOnTestFinished = false;
};



class ISpatialFunctionalTestLBDelegationInterface
{
	GENERATED_BODY()

public:
	bool AddActorDelegation(AActor* Actor, VirtualWorkerId WorkerId, bool bPersistOnTestFinished = false);
	bool RemoveActorDelegation(AActor* Actor);
	bool HasActorDelegation(AActor* Actor);

	void RemoveAllActorDelegations(bool bRemovePersistent = false);

protected:
	TMap<uint32, FSpatialFunctionalTestActorDelegation> Delegations;
};
