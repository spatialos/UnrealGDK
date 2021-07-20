// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "UnresolvedReferenceGymTestActor.generated.h"

/**
 * A Helper actor for the dormancy tests.
 * Has a TestIntProp to see if it replicates when it should/shouldn't.
 */

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUnresolvedReferenceGymTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int numRepNotify;

	UPROPERTY(ReplicatedUsing = OnRep_ActorRefs)
	TArray<AReplicatedTestActorBase*> ActorRefs;

	UFUNCTION()
	void OnRep_ActorRefs();
};