// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "RefreshActorDormancyTestActor.generated.h"

/**
 * A Helper actor for the dormancy tests.
 * Has a TestIntProp to see if it replicates when it should/shouldn't.
 */

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARefreshActorDormancyTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ARefreshActorDormancyTestActor();
};
